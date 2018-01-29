#include <stdio.h>
#include <stdlib.h>

#include "errors.h"
#include "flash_io.h"
#include "zs_data.h"

int main(int argc, char* argv[])
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
    free(flash_RAM);
    printf("save size:  %li\n", file_size);
    return ERR_NONE;
}

/*
 * provided to reduce EXE file size of the MS-DOS build by DJGPP
 *
 * DJGPP      :  gcc -o zs *.c -Os -lc # -lc makes -lgcc get stripped out
 * Strip separately (not passing -s to gcc) with:  strip -s zs.exe
 *
 * Open Watcom:  wcl386 -fe=zs *.c -os -d0 -s -bcl=stub32xc
 */
#ifdef __DJGPP__
#include <crt0.h>

char**
__crt0_glob_function(char* arg)
{
    return NULL;
}
void
__crt0_load_environment_file(char* progname)
{
    return;
}
#endif
