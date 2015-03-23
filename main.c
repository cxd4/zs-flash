#include <stdio.h>
#include "errors.h"
#include "flash_io.h"

int main(int argc, char ** argv)
{
    unsigned int word;

    flash_RAM[0x000004] = 0xDE;
    flash_RAM[0x000005] = 0xAD;
    flash_RAM[0x000006] = 0xBE;
    flash_RAM[0x000007] = 0xEF;

    write64(&flash_RAM[0x002000], &flash_RAM[0x000000]);
    word = read32(&flash_RAM[0x002004]);
    printf("%08X\n", word);

    if (argc < 2)
    {
        my_error(ERR_FLASHRAM_LOCATION_UNKNOWN);
        return ERR_FLASHRAM_LOCATION_UNKNOWN;
    }
    return ERR_NONE;
}
