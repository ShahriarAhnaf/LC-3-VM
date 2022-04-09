#########################################################################################

## Credits
[CREDIT TO JUSTIN MEINER'S INSTRUCTIONS AND TEACHING](https://justinmeiners.github.io/lc3-vm/index.html#1:12)

[The design document for the usage](https://justinmeiners.github.io/lc3-vm/supplies/lc3-isa.pdf)

## Usage

clone repo and cd into directory with terminal. 

```
# compiles the VM 
make VM

# compiles Debugger VM with debug log support
make Debug

# compiles VM with speed log support
make Log

#runs 2048 directly
make run-2048
#
```

**IMPROVEMENTS upon designs!!**
- Opimization results!
- ![FINAL PIC](https://github.com/ShahriarAhnaf/LC-3-VM/blob/main/assets/optimized-diff.png)

**VM PROJECT USING C**

This is the tutorial project to understand computer instruction architecture such as LC3

## UNDERSTANDING THE VM
**Memory**
This is all the memory allocated for the Vm to use, for LC-3 its 65535(UINT16_max)

# LOOKS LIKE THIS 
![design](https://github.com/ShahriarAhnaf/LC-3-VM/blob/main/assets/Design.png)

**Registers**

Theres are The LC-3 has 10 total registers, each of which is 16 bits. Most of them are general purpose, but a few have designated roles.

8 general purpose registers (R0-R7)
1 program counter (PC) register
1 condition flags (COND) register

**Operation Codes**

Each opcode represents one task that the CPU "knows" how to do. There are just **16 opcodes** in LC-3. Everything the computer can calculate is some sequence of these simple instructions. Each instruction is 16 bits long, with the left 4 bits storing the opcode. The rest of the bits are used to store the parameters.

## PROCEDURE

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

**REGISTERS**

The registers are virtual cpu addresses to the memory allocated to the VM. All the registers only contain addresses not the values itself dont be confused!

**HOW does it find opcodes???**
OPcodes are the first 4 bits of the 16 bit instructions system. By moving over the instruction we can get the appropriate opcode!


## OPCODES!

*ADD*
If bit [5] is 0, the second source operand is obtained from SR2. If bit [5] is 1, the second source operand is obtained by sign-extending the imm5 field to 16 bits. In both cases, the second source operand is added to the contents of SR1 and the result stored in DR.

**AND**

If bit [5] is 0, the second source operand is obtained from SR2. If bit [5] is 1,
the second source operand is obtained by sign-extending the imm5 field to 16
bits. In either case, the second source operand and the contents of SR1 are bitwise ANDed, and the result stored in DR. The condition codes are set, based on
whether the binary value produced, taken as a 2’s complement integer, is negative,
zero, or positive.

**NOT**

find destination register for r1 and then apply the bitwise NOT operator to the address data. 

**BRANCH**

- checks the condition of the n(negative), z(zero), p(positive) bits of the in the instruction. 
- finds the last operation and the flag that it finds. if the flag matches what the branch opcode is looking for then it will fork

- BASICALLY HOW TO MAKE AN IF STATEMENT USING ASSEMBLY

**jump and return codes**

- Takes a register to jump to and changes the PC counter to that address
- return is basically the opposite of jump. 
- Register 7 always contains the previous instruction address which the return uses in the form of 0b111 for ret

**JUMP TO SUBROUTINE(JSR)**
- set current counter as R7 
- if the bit 11 is 1 then sign extend 0-10 bits for the jump address
- if the bit is not 1 then jump to address in base address in bits 8-6. 

**LOAD**
- Load a value from 9 bit address that must be extended
- use special mem_read function to read from the address IN THE VM not original PC. 

**LOAD INDIRECT**

- An address is computed by sign-extending bits [8:0] to 16 bits and adding this
- value to the incremented PC. What is stored in memory at this address is the
- address of the data to be loaded into DR. The condition codes are set, based on
- update flag whether the value loaded is negative, zero, or positive.

**LOAD EFFECTIVE ADDR**
- load the direct address as an offset into the PC counter. 
- update the destination register

**LOAD REGISTER**
- loads the address in the register given in bits [11:9] 
- loads from the Base registers + a sign extended offset from the 6 bits at the end. 


**STORE**
- uses the last 9 bits to write an address into the registers from an offset position from the PC

**STORE INDIRECT**
- uses the same principles as the store functions but instaed of storing it in the offset position
- stores the contents of the ADDRESS IN THE OFFSET. MUSt READ THE OFFSET aDDRESS.
**Store register STR**
- reads the contents in the base register + an offset from the 6 last bits and writes it to the address specified in bits [11-9]

**TRAP OPCODES**
- Defined in an enum!
- Basically OS API for how to deal with I/O from the console and how to do basic actions such as keyboard input and outputting into the terminal with characters. 
- INSTEAD OF MAKING FUNCTIONS TRY TO ADD CODE IN THE SWITCH STATEMENT IT WILL BE A FASTER IMPLEMENTATION.

**BIG** VS *LITTLE* ENDIAN
- most computers are little endian however the intsructions for LC-3 is LIttle endian. 
- This means that the instructions when they are being loaded in from memory must be "flipped" for the purpose of the big endian architecture of LC3


**Memory Mapped Registers**
- Some special registers are not accessible from the normal register table. Instead, a special address is reserved for them in memory. To read and write to these registers, you just read and write to their memory location. These are called memory mapped registers. They are commonly used to interact with special hardware devices.
- These registers let the "computer" executing until the keyboard is pressed and THEN you can retrieve the data. instead of using getc and halting the whole computer. 


