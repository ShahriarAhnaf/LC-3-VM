#include "VM.h"

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
