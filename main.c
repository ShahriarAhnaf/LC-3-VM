#include "VM.h"
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
#ifdef DEBUGGER
    MAP_VM();
#endif
    int running = 1;
    while (running)
    {
        uint16_t instr = mem_read(registers[R_PC]++);
#ifdef DEBUGGER
        MAP_REGISTERS();
#endif
        uint16_t op = instr >> 12; // moves over instructions to opcode section
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
                registers[R_PC] += long_pc_offset; /* JSR */
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
        {
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