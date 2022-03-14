##############################################

[CREDIT TO JUSTIN MEINER'S INSTRUCTIONS AND TEACHING](https://justinmeiners.github.io/lc3-vm/index.html#1:12)


**VM PROJECT USING C**

This is the tutorial project to understand computer instruction architecture such as LC3, x86 and ARM


**UNDERSTANDING THE VM**
**Memory**

This is all the memory allocated for the Vm to use, for LC-3 its 65535(UINT16_max)
**Registers**

Theres are The LC-3 has 10 total registers, each of which is 16 bits. Most of them are general purpose, but a few have designated roles.

8 general purpose registers (R0-R7)
1 program counter (PC) register
1 condition flags (COND) register

**Operation Codes**

Each opcode represents one task that the CPU "knows" how to do. There are just *16 opcodes* in LC-3. Everything the computer can calculate is some sequence of these simple instructions. Each instruction is 16 bits long, with the left 4 bits storing the opcode. The rest of the bits are used to store the parameters.

**PROCEDURE**

Here is the procedure we need to write:

1. Load one instruction from memory at the address of the PC register.
2. Increment the PC register.
3. Look at the opcode to determine which type of instruction it should perform.
4. Perform the instruction using the parameters in the instruction.
5. Go back to step 1


*How the VM works*

The reads a bunch of 16 bit instructions and emulates the appropriate hardware instructions(opcodes) by just running them like functions.

The vm.out file can be run with the binary instructions given as the instructions which will be executed in the terminal.
This will let me run the VM like a program for another program.

**HOW does it find opcodes???**
OPcodes are the first 4 bits of the 16 bit instructions system. By moving over the instruction we can get the appropriate opcode!


OPCODES!

*ADD*

If bit [5] is 0, the second source operand is obtained from SR2. If bit [5] is 1, the second source operand is obtained by sign-extending the imm5 field to 16 bits. In both cases, the second source operand is added to the contents of SR1 and the result stored in DR.



***TASKS!***

- FINISH THE OPCODES!
    16 remaining
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
    OP_TRAP 

