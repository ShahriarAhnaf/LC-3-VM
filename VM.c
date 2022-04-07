#include "VM.h"

#ifdef DEBUGGER
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)       \
    (byte & 0x80 ? '1' : '0'),     \
        (byte & 0x40 ? '1' : '0'), \
        (byte & 0x20 ? '1' : '0'), \
        (byte & 0x10 ? '1' : '0'), \
        (byte & 0x08 ? '1' : '0'), \
        (byte & 0x04 ? '1' : '0'), \
        (byte & 0x02 ? '1' : '0'), \
        (byte & 0x01 ? '1' : '0')

void MAP_REGISTERS(void)
{
    printf("\nSnapshot\n");
    for (int i = 0; i < R_COUNT; i++)
    {
        if(i == R_PC){
            printf("Register: %d = %x \n", i, registers[i]);
        }
        else {
        printf("Register: %d ="" m: " BYTE_TO_BINARY_PATTERN " " BYTE_TO_BINARY_PATTERN "\n",i,
                   BYTE_TO_BINARY(registers[i] >> 8), BYTE_TO_BINARY(registers[i]));
        }
    }
}

void MAP_VM(void)
{
    uint16_t addyCounter = 0;
    while (addyCounter < UINT16_MAX)
    {
        uint16_t memory = mem_read(addyCounter);
        if (memory != 0)
        {
            printf("\naddress %#010x : ", addyCounter);
            printf("m: " BYTE_TO_BINARY_PATTERN " " BYTE_TO_BINARY_PATTERN "\n",
                   BYTE_TO_BINARY(memory >> 8), BYTE_TO_BINARY(memory));
        }
        addyCounter++;
    }
}
#else
void MAP_VM(void){

}
void MAP_REGISTERS(void){

}
#endif
