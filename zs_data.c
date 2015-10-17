#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <time.h>

#include "errors.h"
#include "flash_io.h"
#include "zs_data.h"

p_opt * opt_table;

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

int file_erase(int optc, char ** optv)
{
    int response;
    const int skip_warn = (optc > 1) ? 1 : 0;
    const int wipe_mode = (optc > 2) ? ~0x00 : 0x00;

    if (skip_warn)
        if (strcmp(optv[1], "NOCONFIRM") == 0)
            goto skip_confirmation;
    printf("Erasing game data at %p.  Continue?  ", file - flash_RAM);
    response = getchar();
    if (response % 2 == 0)
        return ERR_NONE;

skip_confirmation:
    memset(file, wipe_mode, FILE_SIZE);
    puts("File has been deleted.");
    return ERR_NONE;
}
int file_new(int optc, char ** optv)
{
    u64 filled_doubleword;
    register size_t i;

    srand((unsigned int)time(NULL));
    for (i = 0x0FEC; i < 0x0FF5; i += 1) /* random lottery ticket numbers */
        write8(file + i, (unsigned char)(rand() % 10));
    for (i = 0x0FF5; i < 0x0FFB; i += 1) /* oceanic spider house shoot order */
        write8(file + i, (unsigned char)(rand() % 4));
    for (i = 0x0FFB; i < 0x1000; i += 1) { /* Bombers' password table */
        unsigned char random_digit;
        register size_t j;
fixrand: /* The Bombers' password can't use any digit more than once. */
        random_digit = (unsigned char)(rand() % 5) + 1;
        for (j = 0x0FFB; j < i; j++)
            if (random_digit == read8(file + j))
                goto fixrand;
        write8(file + i, random_digit);
    }

    write64(file + 0x0024, 0x5A454C4441330000); /* magic no. & `savect` reset */
    if (optc < 2)
        return ERR_NONE;

    write32(file + 0x0000, 0x00001C00);
    write8 (file + 0x0005, 0); /* enables opening prologue */
    write32(file + 0x0008, 0x0000FFF0); /* wut... */
    write16(file + 0x000C, 0x3FFF); /* approximately 8 AM */

    write8 (file + 0x0020, 4); /* normal Link, without transformation */
    write16(file + 0x0034, 3 * HEART_HP); write16(file + 0x0036, 3 * HEART_HP);
    write8 (file + 0x0038, 0); write8 (file + 0x0039, 48); /* 48/0 MP??? */

    write8 (file + 0x0044, 0xFF); /* "first_memory" wtf is this */
    write8 (file + 0x0048, 0xFF); /* "last_warp_point" wtf is this */
    write16(file + 0x004A, 0x0008); /* "scene_data_ID" LOLWUT!  never changes */

    for (i = 0x004C; i < 0x005C - 4; i += 4)
        write32(file + i, 0x4DFFFFFF);
    write32(file + 0x0058, 0xFFFFFFFDul);

    write32(file + 0x005C, 0xFFFFFF00ul);
    for (i = 0x005C + 4; i < 0x006C; i += 4)
        write32(file + i, 0xFFFFFFFFul);

    filled_doubleword = 0ul;
    filled_doubleword = ~(filled_doubleword);

    if (toupper(optv[1][0]) == 'K')
        goto skip_inventory_reset;
    write16(file + 0x006C, 0x0011); /* "equip_item" wtf is this */
    for (i = 0x0070; i < 0x00A0; i += 8)
        write64(file + i, filled_doubleword);
    for (i = 0x00A0; i < 0x00B8; i += 8)
        write64(file + i, 0);
    write32(file + 0x00B8, 0x00120000);
    write32(file + 0x00BC, 0x00000000);
skip_inventory_reset:

    for (i = 0x00CA; i < 0x00D4 - 1; i++)
        write8(file + i, 0xFF); /* -1 small keys for 9/10 dungeons...uh, why? */
    write8(file + i, 0x00); /* 0 keys for the 10th...must be a bug in ROM */

 /* "degnuts_memory_name" -- appears to always store 3 copies of player_name */
    filled_doubleword = read64(file + 0x002C);
    for (i = 0x00DE; i < 0x00F6; i += 8)
        write64(file + i, filled_doubleword);

 /* unnamed giant heap of data in the file--no clue what goes on here */
    write32(file + 0x0E6C, 0x1D4C);
    write32(file + 0x0E70, 0x1D4C);
    write32(file + 0x0E74, 0x1DB0);
    write16(file + 0x0EE8, 0x13);
    write16(file + 0x0EEA, 0x0A);
    write16(file + 0x0EEE, 0x00001770);
    write32(file + 0x0EF4, 0x000A0027);

    write16(file + 0x1000, 0x0035); /* "spot_no":  same WTF as scene_data_ID */
    write16(file + 0x1002, 0xFA74); /* Epona's x-coordinate */
    write16(file + 0x1004, 0x0101); /* Epona's y-coordinate */
    write16(file + 0x1006, 0xFAFB); /* Epona's z-coordinate */
    write16(file + 0x1008, 0x2AAC); /* "horse_a" wtf is this. */
    return ERR_NONE;
}
int file_swap(int optc, char ** optv)
{
    u8 * destination_file;
    unsigned long input;
    unsigned int file_page_ID;
    register size_t i;

    if (optc < 2)
        return ERR_INTEGER_COUNT_INSUFFICIENT;
    input = strtoul(optv[1], NULL, 8);

    if (input > 017)
        return ERR_INTEGER_TOO_LARGE;
    file_page_ID = (unsigned int)input;

    destination_file = &flash_RAM[file_page_ID * FILE_SIZE];
    if (destination_file == file)
        return ERR_NONE; /* null operation:  file swapped with itself */

    for (i = 0x0000; i < FILE_SIZE; i += 8) {
        const u64 dst_block = read64(&destination_file[i]);
        const u64 src_block = read64(file + i);

        write64(destination_file + i, src_block);
        write64(file + i, dst_block);
    }
    return ERR_NONE;
}
int file_copy(int optc, char ** optv)
{
    u8 * destination_file;
    unsigned long input;
    unsigned int file_page_ID;
    register size_t i;

    if (optc < 2)
        return ERR_INTEGER_COUNT_INSUFFICIENT;
    input = strtoul(optv[1], NULL, 8);

    if (input > 017)
        return ERR_INTEGER_TOO_LARGE;
    file_page_ID = (unsigned int)input;

    destination_file = &flash_RAM[file_page_ID * FILE_SIZE];
    if (destination_file == file)
        return ERR_NONE; /* null operation:  file copied onto itself */

    for (i = 0x0000; i < FILE_SIZE; i += 8)
        write64(
            &destination_file[i],
            read64(file + i)
        );
    return ERR_NONE;
}

int player_mask(int optc, char ** optv)
{
    unsigned long input;
    const size_t file_offset = 0x000004;

    if (optc < 2)
        return show8("player_mask", file_offset);
    input = strtoul(optv[1], NULL, 0);
    return send8(file_offset, input);
}

int zelda_time(int optc, char ** optv)
{
    unsigned long input;
    const size_t file_offset = 0x00000C;

    if (optc < 2)
        return show16("zelda_time", file_offset);
    input = strtoul(optv[1], NULL, 0);
    return send16(file_offset, input);
}

int change_zelda_time(int optc, char ** optv)
{
    signed long input;
    int sign_extension_failed;
    const size_t file_offset = 0x000014;

    if (optc < 2)
        return show32("change_zelda_time", file_offset);
    input = strtol(optv[1], NULL, 0);
    sign_extension_failed = sendx32(file_offset, input);
    if (sign_extension_failed) /* really 16-bit data, but signed to 32-bit */
        return (sign_extension_failed);
    return sendx16(file_offset + 2, input);
}

int totalday(int optc, char ** optv)
{
    signed long input;
    const size_t file_offset = 0x000018;

    if (optc < 2)
        return show32("totalday", file_offset);
    input = strtol(optv[1], NULL, 0);
    return sendx32(file_offset, input);
}

int player_character(int optc, char ** optv)
{
    unsigned long input;
    const size_t file_offset = 0x000020;

    if (optc < 2)
        return show8("player_character", file_offset);
    input = strtoul(optv[1], NULL, 0);
    return send8(file_offset, input);
}

int bell_flag(int optc, char ** optv)
{
    unsigned long input;
    const size_t file_offset = 0x000022;

    if (optc < 2)
        return show8("bell_flag", file_offset);
    input = strtoul(optv[1], NULL, 0);
    return send8(file_offset, input);
}

int player_name(int optc, char ** optv)
{
    const size_t file_offset = 0x00002C;

    if (optc < 2)
        return show64("player_name", file_offset);
    return send64(file_offset, optv[1]);
}

int life_energy_points(int optc, char ** optv)
{
    signed long input;
    int Boolean;
    const size_t file_offset = 0x000034;

    if (optc < 2)
        return ERR_INTEGER_COUNT_INSUFFICIENT;
    input = strtol(optv[1], NULL, 0); /* Boolean ? now_life : max_life */
    Boolean = (input != 0) ? 1 : 0;
    if (optc < 3)
        return show16(
            Boolean ? "now_life" : "max_life",
            file_offset + 2*Boolean
        );
    input = strtol(optv[2], NULL, 0);
    return sendx16(file_offset + 2*Boolean, input);
}

int magic_points(int optc, char ** optv)
{
    signed long input;
    int Boolean;
    const size_t file_offset = 0x000038;

    if (optc < 2)
        return ERR_INTEGER_COUNT_INSUFFICIENT;
    input = strtol(optv[1], NULL, 0); /* Boolean ? magic_now : magic_max */
    Boolean = (input != 0) ? 1 : 0;
    if (optc < 3)
        return show8(
            Boolean ? "magic_now" : "magix_max",
            file_offset + Boolean
        );
    input = strtol(optv[2], NULL, 0);
    return sendx8(file_offset + Boolean, input);
}

int lupy_count(int optc, char ** optv)
{
    signed long input;
    const size_t file_offset = 0x00003A;

    if (optc < 2)
        return show16("lupy_count", file_offset);
    input = strtol(optv[1], NULL, 0);
    return sendx16(file_offset, input);
}

int long_sword_hp(int optc, char ** optv)
{
    signed long input;
    const size_t file_offset = 0x00003C;

    if (optc < 2)
        return show16("long_sword_hp", file_offset);
    input = strtol(optv[1], NULL, 0);
    return sendx16(file_offset, input);
}

int memory_warp_point(int optc, char ** optv)
{
    unsigned long input;
    const size_t file_offset = 0x000046;

    if (optc < 2)
        return show16("memory_warp_point", file_offset);
    input = strtoul(optv[1], NULL, 2);
    return send16(file_offset, input);
}

int item_register(int optc, char ** optv)
{
    u8 inv[INVENTORY_TABLE_HEIGHT][INVENTORY_TABLE_WIDTH];
    size_t file_offset;
    unsigned long input, x, y;
    int subscreen_type;

    if (optc < 2)
        return ERR_INTEGER_COUNT_INSUFFICIENT;
    subscreen_type = toupper(optv[1][0]);
    file_offset  = 0x000070;
    file_offset += (subscreen_type == 'M') ? 0x0018 : 0x0000;

    for (y = 0; y < INVENTORY_TABLE_HEIGHT; y++)
        for (x = 0; x < INVENTORY_TABLE_WIDTH; x++)
            inv[y][x] = file[file_offset + x + y*INVENTORY_TABLE_WIDTH];

    if (optc < 3 + 1 + 1) { /* argv < "-I (string) (x) (y) (ITEM_ID)" */
        printf(
            "%s (%s):\n"\
            "\t%02X %02X %02X %02X %02X %02X\n"\
            "\t%02X %02X %02X %02X %02X %02X\n"\
            "\t%02X %02X %02X %02X %02X %02X\n"\
            "\t%02X %02X %02X %02X %02X %02X\n",

            "item_register",
            (subscreen_type == 'M') ? "masks" : "items",
            inv[0][0], inv[0][1], inv[0][2], inv[0][3], inv[0][4], inv[0][5],
            inv[1][0], inv[1][1], inv[1][2], inv[1][3], inv[1][4], inv[1][5],
            inv[2][0], inv[2][1], inv[2][2], inv[2][3], inv[2][4], inv[2][5],
            inv[3][0], inv[3][1], inv[3][2], inv[3][3], inv[3][4], inv[3][5]
        );
        return ERR_NONE;
    }
    x = strtoul(optv[2], NULL, 0);
    y = strtoul(optv[3], NULL, 0);

    if (x >= INVENTORY_TABLE_WIDTH || y >= INVENTORY_TABLE_HEIGHT)
        return ERR_INTEGER_TOO_LARGE;
    input = strtoul(optv[4], NULL, 16);
    return send8(file_offset + x + y*INVENTORY_TABLE_WIDTH, input);
}

int item_count(int optc, char ** optv)
{
    unsigned long input;
    signed long count;
    unsigned int item_ID;
    const size_t file_offset = 0x0000A0;

    if (optc < 2)
        return ERR_INTEGER_COUNT_INSUFFICIENT;
    input = strtoul(optv[1], NULL, 0);

    if (input > 0x0017)
        return ERR_INTEGER_TOO_LARGE;
    item_ID = (unsigned int)input;

    if (optc < 3)
        return show8("item_count", file_offset + item_ID);
    count = strtol(optv[2], NULL, 10);
    return sendx8(file_offset + item_ID, count);
}

int non_equip_register(int optc, char ** optv)
{
    unsigned long input;
    const size_t file_offset = 0x0000B8;

    if (optc < 2)
        return show32("non_equip_register", file_offset);
    input = strtoul(optv[1], NULL, 16);
    return send32(file_offset, input);
}

int collect_register(int optc, char ** optv)
{
    unsigned long input;
    const size_t file_offset = 0x0000BC;

    if (optc < 2)
        return show32("collect_register", file_offset);
    input = strtoul(optv[1], NULL, 2);
    return send32(file_offset, input);
}

int key_compass_map(int optc, char ** optv)
{
    unsigned long input;
    unsigned int dungeon_ID;
    const size_t file_offset = 0x0000C0;

    if (optc < 2)
        return ERR_INTEGER_COUNT_INSUFFICIENT;
    input = strtoul(optv[1], NULL, 0);
    if (input > 9) /* 0x00C0:0x00C9 */
        return ERR_INTEGER_TOO_LARGE;

    dungeon_ID = (unsigned int)input;
    if (optc < 3)
        return show8("key_compass_map", file_offset + dungeon_ID);
    input = strtoul(optv[2], NULL, 2);
    return send8(file_offset + dungeon_ID, input);
}

int key_register(int optc, char ** optv)
{
    unsigned long input;
    unsigned int dungeon_ID;
    const size_t file_offset = 0x0000CA;

    if (optc < 2)
        return ERR_INTEGER_COUNT_INSUFFICIENT;
    input = strtoul(optv[1], NULL, 0);
    if (input > 9) /* 0x00CA:0x00D3 */
        return ERR_INTEGER_TOO_LARGE;

    dungeon_ID = (unsigned int)input;
    if (optc < 3)
        return show8("key_register", file_offset + dungeon_ID);
    input = strtoul(optv[2], NULL, 0);
    return send8(file_offset + dungeon_ID, input);
}

int orange_fairy(int optc, char ** optv)
{
    unsigned long input;
    unsigned int dungeon_ID;
    const size_t file_offset = 0x0000D4;

    if (optc < 2)
        return ERR_INTEGER_COUNT_INSUFFICIENT;
    input = strtoul(optv[1], NULL, 0);
    if (input > 9) /* 0x00D4:0x00DD */
        return ERR_INTEGER_TOO_LARGE;

    dungeon_ID = (unsigned int)input;
    if (optc < 3)
        return show8("orange_fairy", file_offset + dungeon_ID);
    input = strtoul(optv[2], NULL, 0);
    return send8(file_offset + dungeon_ID, input);
}

int numbers_table(int optc, char ** optv)
{
    u8 numbers[3][NUMBERS_PER_TICKET];
    unsigned long input;
    unsigned int day, i;
    const size_t file_offset = 0x000FEC;

    if (optc < 2)
        return ERR_INTEGER_COUNT_INSUFFICIENT;
    input = strtoul(optv[1], NULL, 0);
    if (input > 2) /* 0x0FEC:0x0FF4 */
        return ERR_INTEGER_TOO_LARGE;

    day = (unsigned int)input;
    for (i = 0; i < NUMBERS_PER_TICKET; i++)
        numbers[day][i] = read8(&file[file_offset + 3*day + i]);
    if (optc < 5) {
        printf(
            "%s (day %u):  { %u, %u, %u }\n",
            "numbers_table", day + 1,
            numbers[day][0], numbers[day][1], numbers[day][2]
        );
        return ERR_NONE;
    }

    for (i = 0; i < NUMBERS_PER_TICKET; i++) {
        input = strtoul(optv[i + 2], NULL, 0);
        if (input > 0x000000FFul)
            return ERR_INTEGER_TOO_LARGE;
        numbers[day][i] = (u8)input;
    }
    for (i = 0; i < NUMBERS_PER_TICKET; i++)
        write8(&file[file_offset + 3*day + i], numbers[day][i]);
    return ERR_NONE;
}

int magic_number_test(unsigned int section_ID)
{
    const u8 * section;
    u8 bytes[BYTES_IN_MAGIC_NUMBER];
    register size_t i;
    const size_t file_offset = 0x000024;

    section = &flash_RAM[FILE_SIZE * (section_ID & 0xF)];
    for (i = 0; i < BYTES_IN_MAGIC_NUMBER; i++)
        bytes[i] = read8(section + file_offset + i);
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

/*
 * end of save data field functions and documentation
 *
 * These following nine high-level functions are to make well-defined type
 * range conversion easier and optimize the code for small call stack size.
 */
int show8(const char * name, size_t offset)
{
    u8 output;

    output = read8(file + offset);
    printf("%s:  0x%02X\n", name, output);
    return ERR_NONE;
}
int show16(const char * name, size_t offset)
{
    u16 output;

    output = read16(file + offset);
    printf("%s:  0x%04X\n", name, output);
    return ERR_NONE;
}
int show32(const char * name, size_t offset)
{
    u32 output;

    output = read32(file + offset);
    printf("%s:  0x%08X\n", name, output);
    return ERR_NONE;
}
int show64(const char * name, size_t offset)
{
    u64 output;
    u32 words[2];

    output = read64(file + offset);
    words[1] = (u32)(output >>  0);
    words[0] = (u32)(output >> 32);
    printf("%s:  0x%08X%08X\n", name, words[0], words[1]);
    return ERR_NONE;
}

int send8(size_t offset, unsigned long input)
{
    u8 output;

    if (input > 0x000000FFul)
        return ERR_INTEGER_TOO_LARGE;
    output = (u8)input;
    write8(file + offset, output);
    return ERR_NONE;
}
int send16(size_t offset, unsigned long input)
{
    u16 output;

    if (input > 0x0000FFFFul)
        return ERR_INTEGER_TOO_LARGE;
    output = (u16)input;
    write16(file + offset, output);
    return ERR_NONE;
}
int send32(size_t offset, unsigned long input)
{
    u32 output;

#if (ULONG_MAX < 0xFFFFFFFFUL)
#error Non-ISO-conformant `unsigned long` type.
#elif (ULONG_MAX == 0xFFFFFFFFUL)
    if (errno == ERANGE) /* range overflow during strtoul conversion to ULONG */
#else
    if (input > 0xFFFFFFFFul) /* Having a 64-bit `long` is always nice. */
#endif
        return ERR_INTEGER_TOO_LARGE;
    output = (u32)input;
    write32(file + offset, output);
    return ERR_NONE;
}
int send64(size_t offset, char * argument_string)
{
    u64 output, old_output;
    register size_t i;

/*
 * ANSI C does not define a guaranteed 64-bit type.
 *
 * That would not be a problem if the 64-bit Windows ABI wasn't dumb.
 * On virtually any other 64-bit OS, `long` and strtol() are sufficient.
 */
    output = 0x0000000000000000;
    old_output = output;

    for (i = 0; i < strlen(argument_string); i++)
    {
        unsigned char converted = 0;
        const int capital = toupper(argument_string[i]);

        if (i == 1)
            if (argument_string[0] == '0' && capital == 'X')
                continue;
        if (capital >= '0' && capital <= '9')
            converted = (unsigned char)(capital - '0' + 0x0);
        else if (capital >= 'A' && capital <= 'F')
            converted = (unsigned char)(capital - 'A' + 0xA);
        output = 16*output + converted;

        if (output < old_output) /* Check for unsigned overflow. */
            return ERR_INTEGER_TOO_LARGE;
        old_output = output;
    }
    write64(file + offset, output);
    return ERR_NONE;
}

int sendx8(size_t offset, signed long input)
{
    u8 output;

    if (input < -128)
        return ERR_SIGNED_UNDERFLOW;
    if (input > +127)
        return ERR_SIGNED_OVERFLOW;
    output = (u8)((s8)input);
    write8(file + offset, output);
    return ERR_NONE;
}
int sendx16(size_t offset, signed long input)
{
    u16 output;

    if (input < -32768)
        return ERR_SIGNED_UNDERFLOW;
    if (input > +32767)
        return ERR_SIGNED_OVERFLOW;
    output = (u16)((s16)input);
    write16(file + offset, output);
    return ERR_NONE;
}
int sendx32(size_t offset, signed long input)
{
    u32 output;

#if (LONG_MIN < -2147483648) && (LONG_MAX > +2147483647)
    if (input < -2147483648)
        return ERR_SIGNED_UNDERFLOW;
    if (input > +2147483647)
        return ERR_SIGNED_OVERFLOW;
#elif (LONG_MIN == -2147483648) && (LONG_MAX == +2147483647)
    if (input == LONG_MIN && errno == ERANGE)
        return ERR_SIGNED_UNDERFLOW;
    if (input == LONG_MAX && errno == ERANGE)
        return ERR_SIGNED_OVERFLOW;
#else
#error Non-ISO-conformant `long` type.
#endif
    output = (u32)((s32)input);
    write32(file + offset, output);
    return ERR_NONE;
}

int reserved(int optc, char ** optv)
{
    puts(optv[0]);
    optc = optc; /* unused */
    return (optv[0][0] == '-') ?
        ERR_OPTION_NOT_IMPLEMENTED
      : ERR_OPTION_INVALID;
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
        long option_name_as_a_number;
        int not_a_number;

        if (optv[optc][0] == '\0')
            break; /* We've reached the end of the command buffer. */

        option_name_as_a_number = strtol(&optv[optc][1], NULL, 0);
        not_a_number = (option_name_as_a_number == 0) ? 1 : 0;

        if (optv[optc][0] == '-' && not_a_number)
            break;
        ++optc;
    }

    error_signal = opt_table[optv[0][1]](optc, optv);
    if (error_signal != ERR_NONE)
        my_error(error_signal);
    return (optc);
}

void init_options(void)
{
    register size_t i;
    const size_t limit = 1U << CHAR_BIT;

    opt_table = (p_opt *)malloc(limit * sizeof(p_opt));
    for (i = 0; i < limit; i++)
        opt_table[i] = reserved;

    opt_table['D'] = totalday; /* current day number, preferably from 1 to 4 */
    opt_table['F'] = bell_flag; /* enables/disables Tatl */
    opt_table['I'] = item_register; /* Item and Mask Subscreen mappings */
    opt_table['L'] = life_energy_points; /* current H.P. out of max H.P. */
    opt_table['M'] = magic_points; /* current M.P. out of max M.P. */
    opt_table['N'] = player_name; /* Link's name and the file select name */
    opt_table['R'] = long_sword_hp; /* Razor Sword durability */
    opt_table['U'] = non_equip_register; /* permanent equipment upgrades */
    opt_table['Z'] = change_zelda_time; /* (time_rate + 3) time acceleration */

    opt_table['d'] = key_compass_map; /* boss key and dungeon map and compass */
    opt_table['f'] = orange_fairy; /* stray fairies collected per region */
    opt_table['i'] = item_count; /* inventory counts (generally ammo) */
    opt_table['k'] = key_register; /* small keys per temple */
    opt_table['l'] = numbers_table; /* winning lottery numbers for all nights */
    opt_table['m'] = player_mask; /* currently worn mask */
    opt_table['o'] = memory_warp_point; /* activated owl statues */
    opt_table['p'] = player_character; /* current mask transformation */
    opt_table['q'] = collect_register; /* Quest Status sub-screen data */
    opt_table['r'] = lupy_count; /* That means "Rupees". */
    opt_table['z'] = zelda_time; /* daily time:  0x0000 midnight, 0x8000 noon */

/*
 * special-purpose command-line options fundamental to the flash RAM access
 * Nothing here is pertinent to the game's saved progress data itself.
 */
    opt_table['='] = zs_endian_swap_mask;
    opt_table['@'] = zs_file_pointer;

    opt_table['&'] = file_erase;
    opt_table['|'] = file_new;
    opt_table['^'] = file_swap;
    opt_table['~'] = file_copy;
    return;
}
