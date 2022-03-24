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

int restore_input_buffering()
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

// store the registers in an array
uint16_t registers[R_COUNT]; // empty array for how each register is doing

// store the conditions flags
enum
{
    FL_POS = 1 << 0, /* P bascially 1*/
    FL_ZRO = 1 << 1, /* Z bascially 2 */
    FL_NEG = 1 << 2, /* N basicaly 4 */
};

int main(int arg_count, const char *args[]) // this run the program by taking in the arguments from the terminal!
{

    if (arg_count < 2)
    {
        /* show usage string */
        printf("lc3 [image-file1] ...\n");
        exit(2);
    }

    for (int j = 1; j < arg_count; ++j)
    {
        if (!read_image(args[j]))
        {
            printf("failed to load image: %s\n", args[j]);
            exit(1);
        }
    }
    // UNIX SPECIFIC
    signal(SIGINT, handle_interrupt);
    disable_input_buffering(); // disables terminal moment

    // actual code for the procedure

    /* since exactly one condition flag should be set at any given time, set the Z flag */
    registers[R_COND] = FL_ZRO; // set to Z flag

    /* set the PC to starting position */
    /* 0x3000 is the default */
    enum
    {
        PC_START = 0x3000
    };

    registers[R_PC] = PC_START; // program counter starts at the start of the progam

    uint16_t instr = mem_read(registers[R_PC]++);
    uint16_t op = instr >> 12; // moves over instructions to

    switch (op)
    {
    case OP_ADD:
        op_add_f(&instr);
        break;
        // case OP_AND:
        // {
        //     AND, 7
        // }
        // break;
        // case OP_NOT:
        // {
        //     NOT, 7
        // }
        // break;
        // case OP_BR:
        // {
        //     BR, 7
        // }
        // break;
        // case OP_JMP:
        // {
        //     JMP, 7
        // }
        // break;
        // case OP_JSR:
        // {
        //     JSR, 7
        // }
        // break;
        // case OP_LD:
        // {
        //     LD, 7
        // }
        // break;
    case OP_LD_I:

        break;
        // case OP_LDR:
        // {
        //     LDR, 7
        // }
        // break;
        // case OP_LEA:
        // {
        //     LEA, 7
        // }
        // break;
        // case OP_ST:
        // {
        //     ST, 7
        // }
        // break;
        // case OP_STI:
        // {
        //     STI, 7
        // }
        // break;
        // case OP_STR:
        // {
        //     STR, 7
        // }
        // break;
        // case OP_TRAP:
        // // {
        //     TRAP, 8
        // }
        break;
    case OP_RES:
    case OP_RTI:
    default:
        // {
        //     BAD OPCODE, 7
        // }
        break;
    }
    // UNIX SPECIFIC SHUTDOWN
    restore_input_buffering(); // get back the terminal to normal
}

uint16_t sign_extend(uint16_t x, int bit_count)
{
    if ((x >> (bit_count - 1)) & 1)
    {
        x |= (0xFFFF << bit_count);
    }
    return x;
}

// check the left most bit of the number to see if the register address value is 0 or 1
void update_flag(uint16_t rNum)
{
    if (registers[rNum] == 0)
    {
        registers[R_COND] = FL_POS;
    }
    else if (registers[rNum] >> 15)
    {
        registers[R_COND] = FL_NEG;
    }
    else
    {
        registers[R_COND] = FL_ZRO;
    }
}

/* branch */
void op_branch_f(uint16_t instr) /* branch */
{
    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
    uint16_t cond_flag = (instr >> 9) & 0x7;
    if (cond_flag & registers[R_COND]) // if the bits match
    {
        registers[R_PC] += pc_offset; // jump the PC to somewhere with the pc_offset
    }
}
/* add  */
void op_add_f(uint16_t instr)
{
    // destination address moment
    u_int16_t r0 = (instr >> 9) & 0b111;

    u_int16_t sr1 = instr >> 5; // get the 8th bit
    /* whether we are in immediate mode or normal mode */
    uint16_t imm_flag = (instr >> 5) & 0x1;
    if (imm_flag)
    {
        uint16_t imm5 = sign_extend(instr & 0b11111, 5);
        registers[r0] = registers[sr1] + imm5;
    }
    else
    {
        uint16_t sr2 = instr & 0b111;
        registers[r0] = registers[sr2] + registers[sr1];
    }

    update_flag(r0);
}

/* load indirect address into a register*/
void op_ldi_f(uint16_t instr)
{
    /* destination register (DR) */
    uint16_t r0 = (instr >> 9) & 0x7;
    /* PCoffset 9 bits */
    uint16_t pc_offset = sign_extend(instr & 0b111111111, 9);
    /* add pc_offset to the current PC, look at that memory location to get the final address */
    registers[r0] = mem_read(mem_read(registers[R_PC] + pc_offset));
    update_flag(r0);
}

//     OP_AND,         /* bitwise and */
void op_and_f(uint16_t instr)
{
    uint16_t imm_flag = (instr >> 5) & 0x1;
    uint16_t dr = (instr >> 8) & 0b111;
    u_int16_t sr1 = (instr >> 5) & 0b111;

    if (imm_flag)
    {
        registers[dr] = registers[sr1] & sign_extend(instr & 0b11111, 5);
    }
    else
    {
        uint16_t sr2 = instr & 0b111;
        registers[dr] = registers[sr1] & registers[sr2];
    }
    update_flag(dr);
}

//     OP_NOT,         /* bitwise not */
void op_not_f(uint16_t instr)
{
    uint16_t r0 = (instr >> 9) & 0b111;
    uint16_t r1 = (instr >> 6) & 0b111;

    registers[r0] = ~registers[r1]; // bitwise not
    update_flags(r0);
}

//     OP_JMP,         /* jump */
// also the OP_RET code
void op_jmp_f(uint16_t instr)
{
    u_int16_t jump_r = (instr >> 6) & 0b111;
    registers[R_PC] = registers[jump_r];
}

//     OP_JMP_RES,     /* jump register */
void op_jmp_res_f(uint16_t instr)
{
    registers[R_R7] = registers[R_PC]; // set the r7 to current counter address

    // check the 11 bit
    if ((instr >> 11) & 1)
    {
        registers[R_PC] += sign_extend((instr & 0b11111111111), 11); // JSR
    }
    else
    {
        uint16_t jump_r = (instr >> 6) & 0b111; // get the base R
        registers[R_PC] = registers[jump_r];    // JSSR
    }
}

//     OP_LD,          /* load */
//     OP_ST,          /* store */
//     OP_LDR,         /* load register */
//     OP_ST_RES,      /* store register */
//     OP_RTI,         /* unused */
//     OP_ST_I,        /* store indirect */
//     OP_RES,         /* reserved (unused) */
//     OP_LD_EFF_ADDR, /* load effective address */
//     OP_TRAP