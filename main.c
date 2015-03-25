#include <stdio.h>
#include "errors.h"
#include "flash_io.h"

int main(int argc, char ** argv)
{
    unsigned int word;

    write64(&flash_RAM[0x002000], 0x00000000DEADBEEF);
    word = read32(&flash_RAM[0x002004]);
    printf("%08X\n", word);

    if (argc < 2)
    {
        my_error(ERR_FLASHRAM_LOCATION_UNKNOWN);
        return ERR_FLASHRAM_LOCATION_UNKNOWN;
    }
    return ERR_NONE;
}
