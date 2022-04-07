#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>

//
#include "VM.h"

uint16_t swap16(uint16_t bytes)
{
    // since we have the bytes
    // simply shift the bytes according to the center.
    return bytes << 8 | bytes >> 8;
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
int main(int argc, const char* argv[])
{
    if (argc < 2)
    {
        /* show usage string */
        printf("lc3 [image-file1] ...\n");
        exit(2);
    }
    
    for (int j = 1; j < argc; ++j)
    {
        if (!read_image(argv[j]))
        {
            printf("failed to load image: %s\n", argv[j]);
            exit(1);
        }
    }
    signal(SIGINT, handle_interrupt);
    disable_input_buffering();

    /* since exactly one condition flag should be set at any given time, set the Z flag */
    registers[R_COND] = FL_ZRO;

    /* set the PC to starting position */
    /* 0x3000 is the default */
    enum { PC_START = 0x3000 };
    registers[R_PC] = PC_START;

    int running = 1;
    while (running)
    {
        /* FETCH */
        uint16_t instr = mem_read(registers[R_PC]++);
        uint16_t op = instr >> 12;

        switch (op)
        {
            case OP_ADD:
                {
                    /* destination register (DR) */
                    uint16_t r0 = (instr >> 9) & 0x7;
                    /* first operand (SR1) */
                    uint16_t r1 = (instr >> 6) & 0x7;
                    /* whether we are in immediate mode */
                    uint16_t imm_flag = (instr >> 5) & 0x1;
                
                    if (imm_flag)
                    {
                        uint16_t imm5 = sign_extend(instr & 0x1F, 5);
                        registers[r0] = registers[r1] + imm5;
                    }
                    else
                    {
                        uint16_t r2 = instr & 0x7;
                        registers[r0] = registers[r1] + registers[r2];
                    }
                
                    update_flag(r0);
                }
                break;
            case OP_AND:
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t r1 = (instr >> 6) & 0x7;
                    uint16_t imm_flag = (instr >> 5) & 0x1;
                
                    if (imm_flag)
                    {
                        uint16_t imm5 = sign_extend(instr & 0x1F, 5);
                        registers[r0] = registers[r1] & imm5;
                    }
                    else
                    {
                        uint16_t r2 = instr & 0x7;
                        registers[r0] = registers[r1] & registers[r2];
                    }
                    update_flag(r0);
                }
                break;
            case OP_NOT:
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t r1 = (instr >> 6) & 0x7;
                
                    registers[r0] = ~registers[r1];
                    update_flag(r0);
                }
                break;
            case OP_BRANCH:
                {
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    uint16_t cond_flag = (instr >> 9) & 0x7;
                    if (cond_flag & registers[R_COND])
                    {
                        registers[R_PC] += pc_offset;
                    }
                }
                break;
            case OP_JMP:
                {
                    /* Also handles RET */
                    uint16_t r1 = (instr >> 6) & 0x7;
                    registers[R_PC] = registers[r1];
                }
                break;
            case OP_JMP_RES:
                {
                    uint16_t long_flag = (instr >> 11) & 1;
                    registers[R_R7] = registers[R_PC];
                    if (long_flag)
                    {
                        uint16_t long_pc_offset = sign_extend(instr & 0x7FF, 11);
                        registers[R_PC] += long_pc_offset;  /* JSR */
                    }
                    else
                    {
                        uint16_t r1 = (instr >> 6) & 0x7;
                        registers[R_PC] = registers[r1]; /* JSRR */
                    }
                    break;
                }
                break;
            case OP_LD:
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    registers[r0] = mem_read(registers[R_PC] + pc_offset);
                    update_flag(r0);
                }
                break;
            case OP_LD_I:
                {
                    /* destination register (DR) */
                    uint16_t r0 = (instr >> 9) & 0x7;
                    /* PCoffset 9*/
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    /* add pc_offset to the current PC, look at that LeMem location to get the final address */
                    registers[r0] = mem_read(mem_read(registers[R_PC] + pc_offset));
                    update_flag(r0);
                }
                break;
            case OP_LDR:
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t r1 = (instr >> 6) & 0x7;
                    uint16_t offset = sign_extend(instr & 0x3F, 6);
                    registers[r0] = mem_read(registers[r1] + offset);
                    update_flag(r0);
                }
                break;
            case OP_LD_EFF_ADDR:
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    registers[r0] = registers[R_PC] + pc_offset;
                    update_flag(r0);
                }
                break;
            case OP_ST:
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    mem_write(registers[R_PC] + pc_offset, registers[r0]);
                }
                break;
            case OP_ST_I:
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    mem_write(mem_read(registers[R_PC] + pc_offset), registers[r0]);
                }
                break;
            case OP_ST_RES:
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t r1 = (instr >> 6) & 0x7;
                    uint16_t offset = sign_extend(instr & 0x3F, 6);
                    mem_write(registers[r1] + offset, registers[r0]);
                }
                break;
            case OP_TRAP:
                switch (instr & 0xFF)
                {
                    case TRAP_GETCHAR:
                        /* read a single ASCII char */
                        registers[R_R0] = (uint16_t)getchar();
                        update_flag(R_R0);
                        break;
                    case TRAP_OUTPUT:
                        putc((char)registers[R_R0], stdout);
                        fflush(stdout);
                        break;
                    case TRAP_PUTSTRING:
                        {
                            /* one char per word */
                            uint16_t* c = LeMem + registers[R_R0];
                            while (*c)
                            {
                                putc((char)*c, stdout);
                                ++c;
                            }
                            fflush(stdout);
                        }
                        break;
                    case TRAP_INPUT:
                        {
                            printf("Enter a character: ");
                            char c = getchar();
                            putc(c, stdout);
                            fflush(stdout);
                            registers[R_R0] = (uint16_t)c;
                            update_flag(R_R0);
                        }
                        break;
                    case TRAP_PUTSP:
                        {
                            /* one char per byte (two bytes per word)
                               here we need to swap back to
                               big endian format */
                            uint16_t* c = LeMem + registers[R_R0];
                            while (*c)
                            {
                                char char1 = (*c) & 0xFF;
                                putc(char1, stdout);
                                char char2 = (*c) >> 8;
                                if (char2) putc(char2, stdout);
                                ++c;
                            }
                            fflush(stdout);
                        }
                        break;
                    case TRAP_HALT:
                        puts("HALT");
                        fflush(stdout);
                        running = 0;
                        break;
                }
                break;
            case OP_RES:
            case OP_RTI:
            default:
                abort();
                break;
        }
    }
   restore_input_buffering();
}
