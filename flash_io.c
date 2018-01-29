#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "flash_io.h"
#include "errors.h"

u8* flash_RAM;

u8 read8(const void * address)
{
    const void * min_addr;
    const void * max_addr;
    u8 octet;

    min_addr = &flash_RAM[FLASH_MIN_ADDR];
    max_addr = &flash_RAM[FLASH_MAX_ADDR];
    if (address < min_addr || address > max_addr) {
        my_error(ERR_RD_FLASH_ACCESS_VIOLATION);
        return 0x00u; /* N64 RCP reads 0 from un-mapped DRAM. */
    }
    memcpy(&octet, address, sizeof(u8));
    return (octet &= 0xFFu);
}

u16 read16(const void * address)
{
    const u8 * addr;
    u16 halfword; /* MIPS and from the game's point of view */

    addr = (const u8 *)address;
    halfword  = (u16)read8(addr + 0) << 8;
    halfword |= (u16)read8(addr + 1) << 0;
    return (halfword & 0x000000000000FFFFu);
}

u32 read32(const void * address)
{
    const u8 * addr;
    u32 word; /* MIPS and from the game's point of view */

    addr = (const u8 *)address;
    word  = (u32)read16(addr + 0) << 16;
    word |= (u32)read16(addr + 2) <<  0;
    return (word & 0x00000000FFFFFFFFul);
}

u64 read64(const void * address)
{
    const u8 * addr;
    u64 doubleword;

    addr = (const u8 *)address;
    doubleword  = (u64)read32(addr + 0) << 32;
    doubleword |= (u64)read32(addr + 4) <<  0;
    return (doubleword);
}

void write8(void * dst, const u8 src)
{
    const void * min_addr;
    const void * max_addr;

    min_addr = &flash_RAM[FLASH_MIN_ADDR];
    max_addr = &flash_RAM[FLASH_MAX_ADDR];
    if (dst < min_addr || dst > max_addr) {
        my_error(ERR_WR_FLASH_ACCESS_VIOLATION);
        return; /* N64 RCP writes nothing to un-mapped memory. */
    }
    memcpy(dst, &src, sizeof(u8));
    return;
}

void write16(void * dst, const u16 src)
{
    u8 * addr;
    const u8 src_hi = (u8)((src & 0xFF00u) >> 8);
    const u8 src_lo = (u8)((src & 0x00FFu) >> 0);

    addr = (u8 *)dst;
    write8(addr + 0, src_hi);
    write8(addr + 1, src_lo);
    return;
}

void write32(void * dst, const u32 src)
{
    u8 * addr;
    const u16 src_hi = (u16)((src & 0xFFFF0000ul) >> 16);
    const u16 src_lo = (u16)((src & 0x0000FFFFul) >>  0);

    addr = (u8 *)dst;
    write16(addr + 0, src_hi);
    write16(addr + 2, src_lo);
    return;
}

void write64(void * dst, const u64 src)
{
    u8 * addr;
    const u32 src_hi = (u32)((src >> 32) & 0x00000000FFFFFFFFul);
    const u32 src_lo = (u32)((src & 0x00000000FFFFFFFFul) >>  0);

    addr = (u8 *)dst;
    write32(addr + 0, src_hi);
    write32(addr + 4, src_lo);
    return;
}

u8* falloc(size_t flash_size)
{
    void* address;

    address = malloc(flash_size);
    if (address == NULL) {
        my_error(ERR_OUT_OF_MEMORY);
        exit(EXIT_FAILURE);
    }
    return (u8 *)(address);
}

long load_flash(const char * filename)
{
    FILE * stream;
    unsigned long bytes_read;
    long bytes_read_as_long; /* ftell() and fseek() need signed integers. */

    stream = fopen(filename, "rb");
    if (stream == NULL) {
        my_error(ERR_FILE_STREAM_NO_LINK);
        return (bytes_read_as_long = 0);
    }
    if (flash_RAM == NULL)
        flash_RAM = falloc(FLASH_SIZE);

    for (bytes_read = 0; bytes_read < FLASH_SIZE; bytes_read += BLOCK_SIZE) {
        size_t elements_read;

        elements_read = fread(
            &flash_RAM[bytes_read],
            sizeof(u8), BLOCK_SIZE,
            stream
        );
        if (elements_read != BLOCK_SIZE) {
            my_error(ERR_DISK_READ_FAILURE);
            bytes_read += elements_read;
            break;
        }
    }

    while (fclose(stream) != 0)
        my_error(ERR_FILE_STREAM_STUCK);
    if (bytes_read > LONG_MAX) {
        my_error(ERR_OUT_OF_MEMORY);
        bytes_read = LONG_MAX;
    }
    bytes_read_as_long = (long)(bytes_read);
    return (bytes_read_as_long);
}

long save_flash(const char * filename)
{
    FILE * stream;
    unsigned long bytes_sent;
    long bytes_sent_as_long; /* ftell() and fseek() need signed integers. */

    stream = fopen(filename, "wb");
    if (stream == NULL) {
        my_error(ERR_FILE_STREAM_NO_LINK);
        return (bytes_sent_as_long = 0);
    }
    if (flash_RAM == NULL)
        flash_RAM = falloc(FLASH_SIZE);

    for (bytes_sent = 0; bytes_sent < FLASH_SIZE; bytes_sent += BLOCK_SIZE) {
        size_t elements_written;

        elements_written = fwrite(
            &flash_RAM[bytes_sent],
            sizeof(u8), BLOCK_SIZE,
            stream
        );
        if (elements_written != BLOCK_SIZE) {
            my_error(ERR_DISK_WRITE_FAILURE);
            bytes_sent += elements_written;
            break;
        }
    }

    while (fclose(stream) != 0)
        my_error(ERR_FILE_STREAM_STUCK);
    if (bytes_sent > LONG_MAX) {
        my_error(ERR_OUT_OF_MEMORY);
        bytes_sent = LONG_MAX;
    }
    bytes_sent_as_long = (long)(bytes_sent);
    return (bytes_sent_as_long);
}

static const char* endian_types[] = {
    "hardware-accurate",
    "16-bit byte swapped",
    "32-bit halfword-swapped",
    "32-bit byte-swapped",

    "64-bit word-swapped",
    "mixed-endian",
    "64-bit halfword-swapped",
    "64-bit byte-swapped",
};
unsigned int swap_flash(unsigned int interval)
{
    RCP_block RCP;
    unsigned int mask;
    register u32 i, j;

    if (interval != 0) { /* callee-defined endian swap on a fixed interval */
        mask = interval - 1; /* e.g. (interval = 4) for a 32-bit swap */
        goto swap_memory;
    }
    if (flash_RAM == NULL) {
        my_error(ERR_OUT_OF_MEMORY);
        return 0; /* Swapping an empty flash RAM buffer is pointless. */
    }

/*
 * if (interval == 0) then the memory swapping, if any at all, is done based
 * on automatic detection of the current byte order the flash RAM is in.
 */
    i = 0x0000;
    do {
        RCP.block = read64(&flash_RAM[i + 0x0020]);
        if (RCP.block != 0 && ~RCP.block != 0)
            break;
        i += FILE_SIZE;
    } while (i < FLASH_SIZE);
    if (i >= FLASH_SIZE) { /* overflow from searching an empty file */
        mask = get_client_swap_mask();
        printf("Detected %s data.", "completely blank game");
        goto swap_memory;
    }

    switch ((u32)(RCP.block & 0x00000000FFFFFFFFul)) {
    case /* 'ZELD' */0x5A454C44:
        mask = 0;
        break;
    case /* 'EZDL' */0x455A444C:
        mask = 1;
        break;
    case /* 'LDZE' */0x4C445A45:
        mask = 2;
        break;
    case /* 'DLEZ' */0x444C455A:
        mask = 3;
        break;
    default:
        switch ((u32)((RCP.block >> 32) & 0x00000000FFFFFFFFul)) {
        case /* 'ZELD' */0x5A454C44:
            mask = 4;
            break;
        case /* 'DLEZ' */0x444C455A:
            mask = 7;
            break;
        default:
            fprintf(stderr, "Flash formatting damaged or unsupported.\n");
            return (mask = ~0u);
        }
    }
    printf("Detected %s data.", endian_types[mask % 8]);

swap_memory:
    switch (mask) {
    case 0:
        break; /* Swapping big-endian into big-endian is a null operation. */
    case 1:
        for (i = 0; i < FLASH_SIZE; i += 2) {
            for (j = 0; j < 2; j++)
                RCP.bytes[j] = read8(&flash_RAM[i + j]);
            for (j = 0; j < 2; j++)
                write8(&flash_RAM[i + j], RCP.bytes[j ^ 1]);
        }
        break;
    case 2:
        for (i = 0; i < FLASH_SIZE; i += 4) {
            for (j = 0; j < 2; j++)
                RCP.halfwords[j] = read16(&flash_RAM[i + 2*j]);
            for (j = 0; j < 2; j++)
                write16(&flash_RAM[i + 2*j], RCP.halfwords[j ^ 2/2]);
        }
        break;
    case 3:
        for (i = 0; i < FLASH_SIZE; i += 4) {
            for (j = 0; j < 4; j++)
                RCP.bytes[j] = read8(&flash_RAM[i + j]);
            for (j = 0; j < 4; j++)
                write8(&flash_RAM[i + j], RCP.bytes[j ^ 3]);
        }
        break;
    case 4:
        for (i = 0; i < FLASH_SIZE; i += 8) {
            for (j = 0; j < 2; j++)
                RCP.words[j] = read32(&flash_RAM[i + 4*j]);
            for (j = 0; j < 2; j++)
                write32(&flash_RAM[i + 4*j], RCP.words[j ^ 4/4]);
        }
        break;
    case 7:
        for (i = 0; i < FLASH_SIZE; i += 8) {
            for (j = 0; j < 8; j++)
                RCP.bytes[j] = read8(&flash_RAM[i + j]);
            for (j = 0; j < 8; j++)
                write8(&flash_RAM[i + j], RCP.bytes[j ^ 7]);
        }
        break;
    }
    putchar('\n');
    return (mask);
}

unsigned int get_client_swap_mask(void)
{
    RCP_block result;
    u64 block;

    block   = 0x01234567UL;
    block <<= 32;
    block  |= 0x89ABCDEFUL;
    memcpy(&result.bytes[0], &block, sizeof(i64));
    result.words[0] = 0x00000000
      | ((u32)result.bytes[0] << 24)
      | ((u32)result.bytes[1] << 16)
      | ((u32)result.bytes[2] <<  8)
      | ((u32)result.bytes[3] <<  0)
    ;
    switch (result.words[0]) {
    default:  /* fall through */
    case 0x01234567:  return 00;
    case 0x23016745:  return 01;
    case 0x45670123:  return 02;
    case 0x67452301:  return 03;
    case 0x89ABCDEF:  return 04;
    case 0xAB89EFCD:  return 05;
    case 0xCDEF89AB:  return 06;
    case 0xEFCDAB89:  return 07;
    }
}
