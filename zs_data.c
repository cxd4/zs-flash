#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include "errors.h"
#include "flash_io.h"
#include "zs_data.h"

unsigned int swap_mask;

static const u8 newf[BYTES_IN_MAGIC_NUMBER] = {
    0x5A, /* 'Z' */
    0x45, /* 'E' */
    0x4C, /* 'L' */
    0x44, /* 'D' */
    0x41, /* 'A' */
    0x33, /* '3' */
}; /* "ZELDA3" also works but is not bit-exact or as portable. */

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

int life_energy_points(int optc, char ** optv)
{
    u16 output;
    signed long input;
    int Boolean;

    if (optc < 2)
        return ERR_INTEGER_COUNT_INSUFFICIENT;
    input = strtol(optv[1], NULL, 0); /* Boolean ? now_life : max_life */
    Boolean = (input != 0) ? 1 : 0;
    if (optc < 3)
    {
        output = read16(file + (Boolean ? 0x0036 : 0x0034));
        printf("%s:  0x%04X\n", Boolean ? "now_life" : "max_life", output);
        return ERR_NONE;
    }
    input = strtol(optv[2], NULL, 0);
    if (input < -32768)
        return ERR_SIGNED_UNDERFLOW;
    if (input > +32767)
        return ERR_SIGNED_OVERFLOW;
    output = (u16)((s16)input);
    write16(file + (Boolean ? 0x0036 : 0x0034), output);
    return ERR_NONE;
}

int magic_points(int optc, char ** optv)
{
    u8 output;
    signed long input;
    int Boolean;

    if (optc < 2)
        return ERR_INTEGER_COUNT_INSUFFICIENT;
    input = strtol(optv[1], NULL, 0); /* Boolean ? magic_now : magic_max */
    Boolean = (input != 0) ? 1 : 0;
    if (optc < 3)
    {
        output = read8(file + (Boolean ? 0x0039 : 0x0038));
        printf("%s:  0x%02X\n", Boolean ? "magic_now" : "magic_max", output);
        return ERR_NONE;
    }
    input = strtol(optv[2], NULL, 0);
    if (input < -128)
        return ERR_SIGNED_UNDERFLOW;
    if (input > +127)
        return ERR_SIGNED_OVERFLOW;
    output = (u8)((s8)input);
    write8(file + (Boolean ? 0x0039 : 0x0038), output);
    return ERR_NONE;
}

int magic_number_test(unsigned int section_ID)
{
    const u8 * section;
    u8 bytes[BYTES_IN_MAGIC_NUMBER];
    register size_t i;

    section = &flash_RAM[FILE_SIZE * (section_ID & 0xF)];
    for (i = 0; i < BYTES_IN_MAGIC_NUMBER; i++)
        bytes[i] = read8(section + 0x0024 + i);
    return memcmp(bytes, newf, BYTES_IN_MAGIC_NUMBER * sizeof(u8));
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
    for (i = 0; i < BYTES_IN_MAGIC_NUMBER; i++)
        write8(section + 0x0024 + i, newf[i]);

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
    case 'L':
        error_signal = life_energy_points(optc, optv);
        break;
    case 'M':
        error_signal = magic_points(optc, optv);
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
