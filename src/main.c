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
#ifdef LOGGER
    FILE *log_file;
    log_file = fopen("logs/log_speed.txt", "w+");
#endif

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



    //MAP THE VM
    #ifdef DEBUGGER
    FILE *map_file;
    map_file = fopen("logs/MAP.txt", "w+");
    MAP_VM(map_file);
    #endif
    /* since exactly one condition flag should be set at any given time, set the Z flag */
    registers[R_COND] = FL_ZRO;

    /* set the PC to starting position */
    /* 0x3000 is the default */
    enum { PC_START = 0x3000 };
    registers[R_PC] = PC_START;

    int running = 1;
    // get operational times for comparision.
    struct timespec start, end;
    uint16_t r0,r1,r2,imm_flag, imm5;
    uint16_t pc_offset, base_plus_offset, offset;
        
    int bruh= 0;
    while (running)
    {
        /* FETCH */
        uint16_t instr = mem_read(registers[R_PC]++);
        uint16_t op = instr >> 12;
        
        
        #ifdef DEBUGGER
        
        if(op == 0) MAP_REGISTERS(map_file);
        #endif
        
        
        //get time
        clock_gettime(CLOCK_MONOTONIC_RAW, &start);
        // prejumping the move pepelaff
        if (op & 0b0011) r0 = (instr >> 9) & 0x7;
        if (op & 0b0101) r1 = (instr >> 6) & 0x7;
        switch (op)
        {
            case OP_ADD:
                {
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
                    // CAUSES PROBLEMS
                    registers[R_PC] = registers[r1];
                }
                break;
            case OP_JSR:
                {
                    //actually a long PC offset flag, reusing variables for speed.
                    imm_flag = (instr >> 11) & 1;
                    registers[R_R7] = registers[R_PC];
                    if (imm_flag)
                    {
                        pc_offset = sign_extend(instr & 0x7FF, 11);
                        registers[R_PC] += pc_offset;  /* JSR */
                    }
                    else
                    {
                        registers[R_PC] = registers[r1]; /* JSRR */
                    }
                    break;
                }
                break;
            case OP_LD:
                {
                    pc_offset = sign_extend(instr & 0x1FF, 9);
                    registers[r0] = mem_read(registers[R_PC] + pc_offset);
                    update_flags(r0);
                }
                break;
            case OP_LDI:
                {
                    pc_offset = sign_extend(instr & 0x1FF, 9);
                    /* add pc_offset to the current PC, look at that memory location to get the final address */
                    registers[r0] = mem_read(mem_read(registers[R_PC] + pc_offset));
                    update_flags(r0);
                }
                break;
            case OP_LDR:
                {
                    offset = sign_extend(instr & 0x3F, 6);
                    registers[r0] = mem_read(registers[r1] + offset);
                    update_flags(r0);
                }
                break;
            case OP_LEA:
                {
                    pc_offset = sign_extend(instr & 0x1FF, 9);
                    registers[r0] = registers[R_PC] + pc_offset;
                    update_flags(r0);
                }
                break;
            case OP_ST:
                {
                    pc_offset = sign_extend(instr & 0x1FF, 9);
                    mem_write(registers[R_PC] + pc_offset, registers[r0]);
                }
                break;
            case OP_STI:
                {
                    pc_offset = sign_extend(instr & 0x1FF, 9);
                    mem_write(mem_read(registers[R_PC] + pc_offset), registers[r0]);
                }
                break;
            case OP_STR:
                {
                    offset = sign_extend(instr & 0x3F, 6);
                    mem_write(registers[r1] + offset, registers[r0]);
                }
                break;
            case OP_TRAP:
                switch (instr & 0xFF)
                {
                    case TRAP_GETC:
                        /* read a single ASCII char */
                        registers[R_R0] = (uint16_t)getchar();
                        update_flags(R_R0);
                        break;
                    case TRAP_OUT:
                        bruh=1;
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
        
#ifdef LOGGER   
        if (bruh){
            log_to_file(start,end, log_file, op);
         }   
    }
    fclose(log_file);
#else
    } // end of while loop   
#endif
#ifdef DEBUGGER
    fclose(map_file);
#endif
    restore_input_buffering();
}
