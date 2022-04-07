
//includes
#include "VM.h"
void read_image_file(FILE* file)
{
    /* the origin tells us where in memory to place the image */
    uint16_t origin;
    fread(&origin, sizeof(origin), 1, file);
    origin = swap16(origin);

    /* we know the maximum file size so we only need one fread */
    uint16_t max_read = UINT16_MAX - origin;
    uint16_t* p = memory + origin;
    size_t read = fread(p, sizeof(uint16_t), max_read, file);

    /* swap to little endian */
    while (read-- > 0)
    {
        *p = swap16(*p);
        ++p;
    }
}
int read_image(const char* image_path)
{
    FILE* file = fopen(image_path, "rb");
    if (!file) { return 0; };
    read_image_file(file);
    fclose(file);
    return 1;
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
    FILE *log_file;
    log_file = fopen("log_speed.txt", "w+");
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
    // get operational times for comparision.
    struct timespec start, end;
    int bruh= 0;
    while (running)
    {
        /* FETCH */
        uint16_t instr = mem_read(registers[R_PC]++);
        uint16_t op = instr >> 12;
        //get time
        
        
        clock_gettime(CLOCK_MONOTONIC_RAW, &start);
        
        uint16_t r0,r1,r2,imm_flag, imm5;
        uint16_t pc_offset, base_plus_offset;
        switch (op)
        {
            case OP_ADD:
                {
                    /* destination register (DR) */
                    r0 = (instr >> 9) & 0x7;
                    /* first operand (SR1) */
                    r1 = (instr >> 6) & 0x7;
                    /* whether we are in immediate mode */
                    imm_flag = (instr >> 5) & 0x1;
                
                    if (imm_flag)
                    {
                        imm5 = sign_extend(instr & 0x1F, 5);
                        registers[r0] = registers[r1] + imm5;
                    }
                    else
                    {
                        r2 = instr & 0x7;
                        registers[r0] = registers[r1] + registers[r2];
                    }
                
                    update_flags(r0);
                }
                break;
            case OP_AND:
                {
                    r0 = (instr >> 9) & 0x7;
                    r1 = (instr >> 6) & 0x7;
                    imm_flag = (instr >> 5) & 0x1;
                
                    if (imm_flag)
                    {
                        imm5 = sign_extend(instr & 0x1F, 5);
                        registers[r0] = registers[r1] & imm5;
                    }
                    else
                    {
                        r2 = instr & 0x7;
                        registers[r0] = registers[r1] & registers[r2];
                    }
                    update_flags(r0);
                }
                break;
            case OP_NOT:
                {
                    r0 = (instr >> 9) & 0x7;
                    r1 = (instr >> 6) & 0x7;
                
                    registers[r0] = ~registers[r1];
                    update_flags(r0);
                }
                break;
            case OP_BR:
                {
                    pc_offset = sign_extend(instr & 0x1FF, 9);
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
                    r1 = (instr >> 6) & 0x7;
                    registers[R_PC] = registers[r1];
                }
                break;
            case OP_JSR:
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
                        r1 = (instr >> 6) & 0x7;
                        registers[R_PC] = registers[r1]; /* JSRR */
                    }
                    break;
                }
                break;
            case OP_LD:
                {
                    r0 = (instr >> 9) & 0x7;
                    pc_offset = sign_extend(instr & 0x1FF, 9);
                    registers[r0] = mem_read(registers[R_PC] + pc_offset);
                    update_flags(r0);
                }
                break;
            case OP_LDI:
                {
                    /* destination register (DR) */
                    r0 = (instr >> 9) & 0x7;
                    /* PCoffset 9*/
                    pc_offset = sign_extend(instr & 0x1FF, 9);
                    /* add pc_offset to the current PC, look at that memory location to get the final address */
                    registers[r0] = mem_read(mem_read(registers[R_PC] + pc_offset));
                    update_flags(r0);
                }
                break;
            case OP_LDR:
                {
                    r0 = (instr >> 9) & 0x7;
                    r1 = (instr >> 6) & 0x7;
                    pc_offset = sign_extend(instr & 0x3F, 6);
                    registers[r0] = mem_read(registers[r1] + pc_offset);
                    update_flags(r0);
                }
                break;
            case OP_LEA:
                {
                    r0 = (instr >> 9) & 0x7;
                    pc_offset = sign_extend(instr & 0x1FF, 9);
                    registers[r0] = registers[R_PC] + pc_offset;
                    update_flags(r0);
                }
                break;
            case OP_ST:
                {
                    r0 = (instr >> 9) & 0x7;
                    pc_offset = sign_extend(instr & 0x1FF, 9);
                    mem_write(registers[R_PC] + pc_offset, registers[r0]);
                }
                break;
            case OP_STI:
                {
                    r0 = (instr >> 9) & 0x7;
                    pc_offset = sign_extend(instr & 0x1FF, 9);
                    mem_write(mem_read(registers[R_PC] + pc_offset), registers[r0]);
                }
                break;
            case OP_STR:
                {
                    r0 = (instr >> 9) & 0x7;
                    r1 = (instr >> 6) & 0x7;
                     pc_offset = sign_extend(instr & 0x3F, 6);
                    mem_write(registers[r1] + pc_offset, registers[r0]);
                }
                break;
            case OP_TRAP:
                switch (instr & 0xFF)
                {
                    case TRAP_GETC:
                        /* read a single ASCII char */
                        bruh =1;
                        registers[R_R0] = (uint16_t)getchar();
                        update_flags(R_R0);
                        break;
                    case TRAP_OUT:
                        putc((char)registers[R_R0], stdout);
                        fflush(stdout);
                        break;
                    case TRAP_PUTS:
                        {
                            /* one char per word */
                            uint16_t* c = memory + registers[R_R0];
                            while (*c)
                            {
                                putc((char)*c, stdout);
                                ++c;
                            }
                            fflush(stdout);
                        }
                        break;
                    case TRAP_IN:
                        {
                            printf("Enter a character: ");
                            char c = getchar();
                            putc(c, stdout);
                            fflush(stdout);
                            registers[R_R0] = (uint16_t)c;
                            update_flags(R_R0);
                        }
                        break;
                    case TRAP_PUTSP:
                        {
                            /* one char per byte (two bytes per word)
                               here we need to swap back to
                               big endian format */
                            uint16_t* c = memory + registers[R_R0];
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
        // time to traverse the switch statement.
        if (bruh){
            log_to_file(start,end, log_file, op);
         }   
    }
    fclose(log_file);
    restore_input_buffering();
}
