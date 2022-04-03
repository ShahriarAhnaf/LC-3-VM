#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
/* unix */
#include <unistd.h>
#include <fcntl.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/termios.h>
#include <sys/mman.h>

#ifdef DEBUGGER
void MAP_VM(void);
void MAP_REGISTERS(void);
#endif

// declarations
void mem_write(uint16_t address, uint16_t valoo);
void update_flag(uint16_t instr);
// ALLOACATED   LeMem
uint16_t LeMem[UINT16_MAX];

// ALL THE INFORMATION ABOUT THE TEMPORARY REGISTER THAT IS INSIDE THE "VM" BEING PROCESSED AT THE MOMENT
enum
{
    R_R0 = 0,
    R_R1,
    R_R2,
    R_R3,
    R_R4,
    R_R5,
    R_R6,
    R_R7,   // previous jump instruction holder
    R_PC,   /* program counter */
    R_COND, // last calculation sign
    R_COUNT
};

// OP CODES FOR VIRTUAL CPU, BASICALLY INSTRUCTIONS FOR THE CPU
enum
{
    OP_BRANCH = 0,  /* branch */
    OP_ADD,         /* add  */
    OP_LD,          /* load */
    OP_ST,          /* store */
    OP_JMP_RES,     /* jump register */
    OP_AND,         /* bitwise and */
    OP_LDR,         /* load register */
    OP_ST_RES,      /* store register */
    OP_RTI,         /* unused */
    OP_NOT,         /* bitwise not */
    OP_LD_I,        /* load indirect */
    OP_ST_I,        /* store indirect */
    OP_JMP,         /* jump */
    OP_RES,         /* reserved (unused) */
    OP_LD_EFF_ADDR, /* load effective address */
    OP_TRAP         /* execute trap */
};

// TRAP CODES
enum
{
    TRAP_GETCHAR = 0x20,   /* get character from keyboard, not echoed onto the terminal */
    TRAP_OUTPUT = 0x21,    /* output a character */
    TRAP_PUTSTRING = 0x22, /* output a word string */
    TRAP_INPUT = 0x23,     /* get character from keyboard, echoed onto the terminal */
    TRAP_PUTSP = 0x24,     /* output a byte string */
    TRAP_HALT = 0x25       /* halt the program */
};

//  LeMem MAPPED REGISTERS
enum
{
    MR_KBStatusRegister = 0xFE00, /* keyboard status */
    MR_KBDataRegister = 0xFE02    /* keyboard data */
};
// store the registers in an array
uint16_t registers[R_COUNT]; // empty array for how each register is doing

// store the conditions flags
enum
{
    FL_POS = 1 << 0, /* P bascially 1*/
    FL_ZRO = 1 << 1, /* Z bascially 2 */
    FL_NEG = 1 << 2, /* N basicaly 4 */
};
uint16_t swap16(uint16_t bytes);
void read_image_file(FILE *file);
int read_image(const char *image_path);

uint16_t sign_extend(uint16_t x, int bit_count);

// check the left most bit of the number to see if the register address value is 0 or 1
void update_flag(uint16_t rNum);
// check key needed for memory access
uint16_t check_key();
void mem_write(uint16_t address, uint16_t valoo);
// only reads from the address of VM memory
//  reading keyboard memory calls get char
uint16_t mem_read(uint16_t address);
