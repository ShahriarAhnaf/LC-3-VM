##############################################

{CREDIT TO JUSTIN MEINER'S INSTRUCTIONS AND TEACHING}[https://justinmeiners.github.io/lc3-vm/index.html#1:12]


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
*How to launch VM*

*How to develop programs for the VM* 
