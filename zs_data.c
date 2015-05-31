#include <memory.h>
#include <stdio.h>
#include "errors.h"
#include "flash_io.h"
#include "zs_data.h"

int magic_number_test(unsigned int section_ID)
{
    const u8 * section;
    u8 bytes[6];
    register size_t i;

    section = &flash_RAM[FILE_SIZE * (section_ID & 0xF)];
    for (i = 0; i < sizeof(bytes); i++)
        bytes[i] = read8(section + 0x0024 + i);
    return memcmp(bytes, "ZELDA3", 6);
}

u16 fix_checksum(unsigned int section_ID)
{
    u8 * section;
    register size_t i;
    register u16 checksum, checksum_JAP;

    section = &flash_RAM[FILE_SIZE * (section_ID & 0xF)];
    checksum = 0x0000;

    for (i = 0; i < 0x100A; i++) /* USA and EUR ROMs access the sum here. */
        checksum += read8(section + i);
    write16(section + i, checksum);

    checksum_JAP = checksum;

    for (     ; i < 0x138E; i++) /* JAP ROMs access the sum here. */
        checksum_JAP += read8(section + i);
    write16(section + i, checksum_JAP);

    return (checksum);
}

int opt_execute(char ** optv)
{
    int optc;

    optc = 1;
    if (optv[0][0] != '-')
    {
        my_error(ERR_OPTION_INVALID);
        return (optc);
    }

    while (optv[optc] != NULL)
    {
        if (optv[optc][0] == '\0')
            break; /* We've reached the end of the command buffer. */
        if (optv[optc][0] == '-')
            break;
        ++optc;
    }

    switch (optv[0][1])
    {
    default:
        my_error(ERR_OPTION_NOT_IMPLEMENTED);
        break;
    }
    return (optc);
}
