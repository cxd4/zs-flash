#include <stdio.h>
#include "errors.h"
#include "flash_io.h"

int main(int argc, char ** argv)
{
    long file_size;
    unsigned int swap_mask;
    u32 word;

    write64(&flash_RAM[0x002000], 0x1337C0DEDEADBEEF);
    word = read32(&flash_RAM[0x002004]);
    printf("%08X\n", word);
    word = read32(&flash_RAM[0x002000]);
    printf("%08X\n", word);

    if (argc < 2)
    {
        my_error(ERR_FLASHRAM_LOCATION_UNKNOWN);
        return ERR_FLASHRAM_LOCATION_UNKNOWN;
    }

    file_size = load_flash(argv[1]);
    printf("load size:  %li\n", file_size);

    swap_mask = swap_flash(0);
    if (swap_mask == ~0u)
    {
        my_error(ERR_MEMORY_FORMAT_UNKNOWN);
        return ERR_MEMORY_FORMAT_UNKNOWN;
    }
/*
 * Save-editing work goes here.
 * To do:  Give the user a command-line option to override the swap mask.
 */
    swap_flash(swap_mask + 1);

    file_size = save_flash(argv[1]);
    printf("save size:  %li\n", file_size);
    return ERR_NONE;
}
