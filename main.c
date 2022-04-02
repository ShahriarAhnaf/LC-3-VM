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

void MAP_VM()
{
}
uint16_t swap16(uint16_t bytes)
{
    // since we have the bytes
    // simply shift the bytes according to the center.
    return bytes >> 8 | bytes << 8;
}
void read_image_file(FILE *file)
{
    /* the origin tells us where in LeMem to place the image */
    uint16_t origin; // creating the start of the VM instructions
    // reading the file from the PC address of "origin"
    // reading one instruction at a time
    // and reading 16 bits at a time- > size of origin
    fread(&origin, sizeof(origin), 1, file);
    origin = swap16(origin); // swapping to big endian

    /* we know the maximum file size so we only need one fread */
    uint16_t max_read = UINT16_MAX - origin;
    uint16_t *ptr_to_read_data = LeMem + origin;
    size_t read = fread(ptr_to_read_data, sizeof(uint16_t), max_read, file);

    /* swap to little endian */
    while (read-- > 0)
    {
        *ptr_to_read_data = swap16(*ptr_to_read_data);
        ++ptr_to_read_data;
    }
}
int read_image(const char *image_path)
{
    FILE *file = fopen(image_path, "rb");
    if (!file) // cant read file
    {
        return 0;
    };
    read_image_file(file);
    fclose(file);
    return 1;
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
// check key needed for memory access
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
void mem_write(uint16_t address, uint16_t valoo)
{
    LeMem[address] = valoo;
}
// only reads from the address of VM memory
//  reading keyboard memory calls get char
uint16_t mem_read(uint16_t address)
{
    if (address == MR_KBStatusRegister)
    {
        if (check_key())
        {
            LeMem[MR_KBStatusRegister] = (1 << 15);
            LeMem[MR_KBDataRegister] = getchar();
        }
        else
        {
            LeMem[MR_KBStatusRegister] = 0;
        }
    }
    return LeMem[address];
}

///////////////////////// UNIX SPECIFIC CODES ////////////////////////////////

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

    int running = 1;
    while (running)
    {
        uint16_t instr = mem_read(registers[R_PC]++);
        uint16_t op = instr >> 12; // moves over instructions to opcode section
        switch (op)
        {
        case OP_ADD:
        { // destination address moment
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
        break;
        case OP_AND:
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
        break;
        case OP_NOT:
        {

            uint16_t r0 = (instr >> 9) & 0b111;
            uint16_t r1 = (instr >> 6) & 0b111;

            registers[r0] = ~registers[r1]; // bitwise not
            update_flags(r0);
        }
        break;
        case OP_BRANCH:
        {
            uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
            uint16_t cond_flag = (instr >> 9) & 0x7;
            if (cond_flag & registers[R_COND]) // if the bits match
            {
                registers[R_PC] += pc_offset; // jump the PC to somewhere with the pc_offset
            }
            break;
        }
        case OP_JMP:
        {
            u_int16_t jump_r = (instr >> 6) & 0b111;
            registers[R_PC] = registers[jump_r];
            break;
        }
        case OP_JMP_RES:
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
            break;
        }
        case OP_LD:
        {
            uint16_t r0 = (instr >> 9) & 0b111;
            uint16_t pc_offset = sign_extend(instr & 0b111111111, 9);
            registers[r0] = mem_read(registers[R_PC] + pc_offset);
            update_flag(r0);
            break;
        }
        case OP_LD_I:
        { /* destination register (DR) */
            uint16_t r0 = (instr >> 9) & 0x7;
            /* PCoffset 9 bits */
            uint16_t pc_offset = sign_extend(instr & 0b111111111, 9);
            /* add pc_offset to the current PC, look at that    LeMem location to get the final address */
            registers[r0] = mem_read(mem_read(registers[R_PC] + pc_offset));
            update_flag(r0);
            break;
        }
        case OP_LDR:
        {
            uint16_t r0 = (instr >> 9) & 0b111;                 // destination registers
            uint16_t r1 = (instr >> 6) & 0b111;                 // base registers
            uint16_t offset = sign_extend(instr & 0b111111, 6); // offset address
            registers[r0] = mem_read(registers[r1] + offset);
            update_flag(r0);
            break;
        }
        case OP_LD_EFF_ADDR:
        {
            uint16_t r0 = (instr >> 9) & 0b111;
            registers[r0] = registers[R_PC] + sign_extend(instr & 0b111111111, 9);
            update_flag(r0);
            break;
        }
        case OP_ST:
        {
            uint16_t r0 = (instr >> 9) & 0b111;
            uint16_t pc_offset = sign_extend(instr & 0b111111111, 9);
            mem_write(registers[R_PC] + pc_offset, registers[r0]);
            break;
        }

        case OP_ST_I:
        {
            uint16_t sr = (instr >> 9) & 0b111;
            uint16_t offset = sign_extend(instr & 0b111111111, 9);
            mem_write(mem_read(registers[R_PC] + offset), registers[sr]); // write the content into the sr register.

            break;
        }
        case OP_ST_RES:
        {
            uint16_t sr = (instr >> 9) & 0b111;
            uint16_t baseR = (instr >> 6) & 0b111;
            uint16_t offset = sign_extend(instr & 0b111111, 6);
            memwrite(registers[baseR] + offset, registers[sr]);
            break;
        }
        case OP_TRAP:
        {
            switch (instr & 0xFF)
            {
            case TRAP_GETCHAR:
            {
                registers[R_R0] = (uint16_t)getchar();
                update_flag(R_R0);
            }
            break;
            case TRAP_OUTPUT:
            {
                putc((char)registers[R_R0], stdout);
                fflush(stdout);
            }
            break;
            case TRAP_PUTSTRING:
            {
                /* one char per word */
                // pointer to a char
                uint16_t *ch = LeMem + registers[R_R0]; // using the address of the starting    LeMem to find the register addy
                while (*ch)                             // deref the char when the char is null character at the end it will terminate
                {
                    putc((char)*ch, stdout);
                    ++ch;
                }
                fflush(stdout);
            }
            break;
            case TRAP_INPUT:
            {
                printf("Enter a character: ");
                char c = getchar(); // read keyboard
                putc(c, stdout);    // show to the person that the char has been read
                fflush(stdout);
                registers[R_R0] = (uint16_t)c;
                update_flags(R_R0);
            }
            break;
            case TRAP_PUTSP:
            {
                uint16_t *ch = LeMem + registers[R_R0]; // pointer to the output string
                while (*ch)                             // while char is not null character
                {
                    uint16_t character1 = *ch & 0xFF;
                    putc(character1, stdout);
                    uint16_t character2 = *ch >> 8;
                    if (character2) // if the thing is not null
                    {
                        putc(character2, stdout);
                    }
                    ++ch;
                }
                fflush(stdout);
            }
            break;
            case TRAP_HALT:
            {
                puts("HALT!");
                fflush(stdout);
                running = 0;
            }
            break;
            }
            break;
        }
        case OP_RES:
        case OP_RTI:
        default:
            abort();
            break;
        }
    }
    // UNIX SPECIFIC SHUTDOWN
    restore_input_buffering(); // get back the terminal to normal
    return 0;
}