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

///////////////////////// UNIX SPECIFIC CODES ////////////////////////////////
uint16_t check_key()
{
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    return select(1, &readfds, NULL, NULL, &timeout) != 0;
}
struct termios original_tio;

void disable_input_buffering()
{
    tcgetattr(STDIN_FILENO, &original_tio);
    struct termios new_tio = original_tio;
    new_tio.c_lflag &= ~ICANON & ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
}

void restore_input_buffering()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &original_tio);
}
void handle_interrupt(int signal)
{
    restore_input_buffering();
    printf("\n");
    exit(-2);
}
//////////////////////////////////////////////////////////////////////

// ALLOACATED MEMORY
uint16_t LeMem[UINT16_MAX];

// ALL THE REGISTERS PRESENT IN THE VM
enum
{
    R_R0 = 0,
    R_R1,
    R_R2,
    R_R3,
    R_R4,
    R_R5,
    R_R6,
    R_R7,
    R_PC, /* program counter */
    R_COND,
    R_COUNT
};

// OP CODES FOR VIRTUAL CPU
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

// store the registers in an array
uint16_t registers[R_COUNT];

// store the conditions flags
enum
{
    FL_POS = 1 << 0, /* P bascially 1*/
    FL_ZRO = 1 << 1, /* Z bascially 2 */
    FL_NEG = 1 << 2, /* N basicaly 4 */
};

int main(int arg_count, const char *args[]) // this run the program by taking in the arguments from the terminal!
{
    // UNIX SPECIFIC
    signal(SIGINT, handle_interrupt);
    disable_input_buffering(); // disables terminal moment

    // UNIX SPECIFIC SHUTDOWN
    restore_input_buffering(); // get back the terminal to normal
}