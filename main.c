#include <stdio.h>
#include <stdlib.h>

#include "errors.h"
#include "flash_io.h"
#include "zs_data.h"

int main(int argc, char ** argv)
{
    long file_size;
    unsigned int section_ID;
    register int i;

    if (argc < 2) {
        my_error(ERR_FLASHRAM_LOCATION_UNKNOWN);
        return ERR_FLASHRAM_LOCATION_UNKNOWN;
    }

    file_size = load_flash(argv[1]);
    printf("load size:  %li\n", file_size);

    swap_mask = swap_flash(0);
    if (swap_mask == ~0u)
        my_error(ERR_MEMORY_FORMAT_UNKNOWN);
#if 0
    for (i = 0; i < NUMBER_OF_DATA_FILES; i++)
        printf(
            "%02u.  (JAP, USA, fixed) = (0x%04X, 0x%04X, 0x%04X)\n",
            i,
            read16(flash_RAM + FILE_SIZE*i + 0x138E),
            read16(flash_RAM + FILE_SIZE*i + 0x100A),
            fix_checksum(i)
        )
    ;
#endif

    i = 2; /* The first two arguments can't be part of editing the save data. */
    init_options();
    while (i < argc)
        i += opt_execute(&argv[i]);
    free(opt_table);

    section_ID = (unsigned int)(file - flash_RAM) / FILE_SIZE;
    printf(
        "Saved checksum 0x%04X to section %u.\n",
        fix_checksum(section_ID), section_ID
    );

    swap_flash(swap_mask + 1);
    file_size = save_flash(argv[1]);
    printf("save size:  %li\n", file_size);
    return ERR_NONE;
}
