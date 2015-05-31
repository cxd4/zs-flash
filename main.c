#include <stdio.h>
#include "errors.h"
#include "flash_io.h"
#include "zs_data.h"

int main(int argc, char ** argv)
{
    long file_size;
    unsigned int swap_mask;
    register int i;

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

    i = 2; /* The first two arguments can't be part of editing the save data. */
    while (i < argc)
        i += opt_execute(&argv[i]);

    for (i = 0; i < 16; i++)
        if (magic_number_test(i) == 0)
            printf(
                "Saved checksum 0x%04X to section %i.\n",
                fix_checksum(i), i
            );

/*
 * To do:  Give the user a command-line option to override the swap mask.
 */
    swap_flash(swap_mask + 1);
    file_size = save_flash(argv[1]);
    printf("save size:  %li\n", file_size);
    return ERR_NONE;
}
