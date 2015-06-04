#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include "errors.h"
#include "flash_io.h"
#include "zs_data.h"

unsigned int swap_mask;

/*
 * Unless the user specifies otherwise on the command line, the save file
 * address will by default be File 1 in the flash RAM:  0x2000 * 0.
 */
u8 * file = &flash_RAM[0 * FILE_SIZE];

int player_mask(int optc, char ** optv)
{
    u8 output;
    unsigned long input;

    if (optc < 2)
    {
        output = read8(file + 0x0004);
        printf("%s:  0x%02X\n", "player_mask", output);
        return ERR_NONE;
    }
    input = strtoul(optv[1], NULL, 16);
    if (input > 0xFF)
        return ERR_INTEGER_TOO_LARGE;
    output = (u8)input;
    write8(file + 0x0004, output);
    return ERR_NONE;
}

int player_character(int optc, char ** optv)
{
    u8 output;
    unsigned long input;

    if (optc < 2)
    {
        output = read8(file + 0x0020);
        printf("%s:  0x%02X\n", "player_character", output);
        return ERR_NONE;
    }
    input = strtoul(optv[1], NULL, 0);
    if (input > 0xFF)
        return ERR_INTEGER_TOO_LARGE;
    output = (u8)input;
    write8(file + 0x0020, output);
    return ERR_NONE;
}

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

/*
 * Storing a save file checksum implies we are writing valid save file data,
 * so we may as well make sure that the required magic number is set.
 */
    write8(section + 0x0024, 'Z');
    write8(section + 0x0025, 'E');
    write8(section + 0x0026, 'L');
    write8(section + 0x0027, 'D');
    write8(section + 0x0028, 'A');
    write8(section + 0x0029, '3');

    for (i = 0; i < 0x100A; i++) /* USA and EUR ROMs access the sum here. */
        checksum += read8(section + i);
    write16(section + i, checksum);

    checksum_JAP = checksum;

    for (     ; i < 0x138E; i++) /* JAP ROMs access the sum here. */
        checksum_JAP += read8(section + i);
    write16(section + i, checksum_JAP);

    return (checksum);
}

int zs_endian_swap_mask(int optc, char ** optv)
{
    unsigned int output;
    unsigned long input;

    if (optc < 2)
        return ERR_INTEGER_COUNT_INSUFFICIENT;
    input = strtoul(optv[1], NULL, 0);
    if (input > 0x7) /* Only endianness within a 64-bit boundary is relevant. */
        return ERR_INTEGER_TOO_LARGE;
    output = (unsigned int)input;
    swap_mask = output;
    return ERR_NONE;
}

int zs_file_pointer(int optc, char ** optv)
{
    unsigned int output;
    unsigned long input;

    if (optc < 2)
        return ERR_INTEGER_COUNT_INSUFFICIENT;
    input = strtoul(optv[1], NULL, 0);
    if (input > 0xF) /* Only 16 sections exist. */
        return ERR_INTEGER_TOO_LARGE;

    output = (unsigned int)(file - flash_RAM) / FILE_SIZE;
    printf(
        "Fixed file %u checksum to 0x%04X ",
        output, fix_checksum(output)
    );

    output = (unsigned int)input;
    file = &flash_RAM[FILE_SIZE * output];
    printf("(&flash_RAM[%04X] = %p).\n", FILE_SIZE * output, file);
    return ERR_NONE;
}

int opt_execute(char ** optv)
{
    int error_signal;
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

    error_signal = ERR_NONE;
    switch (optv[0][1])
    {
    case 'm':
        error_signal = player_mask(optc, optv);
        break;
    case 'p':
        error_signal = player_character(optc, optv);
        break;

    case '0':
        error_signal = zs_endian_swap_mask(optc, optv);
        break;
    case '1':
        error_signal = zs_file_pointer(optc, optv);
        break;
    default:
        my_error(ERR_OPTION_NOT_IMPLEMENTED);
        break;
    }
    if (error_signal != ERR_NONE)
        my_error(error_signal);
    return (optc);
}
