#include "VM.h"

uint16_t sign_extend(uint16_t x, int bit_count)
{
    if ((x >> (bit_count - 1)) & 1) {
        x |= (0xFFFF << bit_count);
    }
    return x;
}

uint16_t swap16(uint16_t x)
{
    return (x << 8) | (x >> 8);
}

void update_flags(uint16_t r)
{
    if (registers[r] == 0)
    {
        registers[R_COND] = FL_ZRO;
    }
    else if (registers[r] >> 15) /* a 1 in the left-most bit indicates negative */
    {
        registers[R_COND] = FL_NEG;
    }
    else
    {
        registers[R_COND] = FL_POS;
    }
}

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

void mem_write(uint16_t address, uint16_t val)
{
    memory[address] = val;
}

uint16_t mem_read(uint16_t address)
{
    if (address == MR_KBSR)
    {
        if (check_key())
        {
            memory[MR_KBSR] = (1 << 15);
            memory[MR_KBDR] = getchar();
        }
        else
        {
            memory[MR_KBSR] = 0;
        }
    }
    return memory[address];
}

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

void MAP_VM(FILE* map_file)
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
void MAP_VM(FILE* map_file){

}
void MAP_REGISTERS(FILE* map_file){

}
#endif

#ifdef LOGGER
void log_to_file(struct timespec start, struct timespec end, FILE* log_file, uint16_t op){
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    long long delta_ns = (end.tv_nsec - start.tv_nsec) ;
    fprintf(log_file, "op: %x, time elapsed: %llu\n", op, delta_ns);
}
#else

void log_to_file(struct timespec start, struct timespec end, FILE* log_file, uint16_t op)
{

} 
#endif
