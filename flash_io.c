#include <stdio.h>
#include <string.h>
#include "flash_io.h"
#include "errors.h"

u8 flash_RAM[FLASH_SIZE];

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

long load_flash(const char * filename)
{
    FILE * stream;
    long bytes_read;

    stream = fopen(filename, "rb");
    if (stream == NULL) {
        my_error(ERR_FILE_STREAM_NO_LINK);
        return (bytes_read = 0);
    }

    for (bytes_read = 0; bytes_read < FLASH_SIZE; bytes_read += BLOCK_SIZE) {
        size_t elements_read;

        elements_read = fread(
            &flash_RAM[bytes_read],
            sizeof(u8), BLOCK_SIZE,
            stream
        );
        if (elements_read != BLOCK_SIZE) {
            my_error(ERR_DISK_READ_FAILURE);
            bytes_read = (long)elements_read;
            return (bytes_read);
        }
    }

    while (fclose(stream) != 0)
        my_error(ERR_FILE_STREAM_STUCK);
    return (bytes_read);
}

long save_flash(const char * filename)
{
    FILE * stream;
    long bytes_sent;

    stream = fopen(filename, "wb");
    if (stream == NULL) {
        my_error(ERR_FILE_STREAM_NO_LINK);
        return (bytes_sent = 0);
    }

    for (bytes_sent = 0; bytes_sent < FLASH_SIZE; bytes_sent += BLOCK_SIZE) {
        size_t elements_written;

        elements_written = fwrite(
            &flash_RAM[bytes_sent],
            sizeof(u8), BLOCK_SIZE,
            stream
        );
        if (elements_written != BLOCK_SIZE) {
            my_error(ERR_DISK_WRITE_FAILURE);
            bytes_sent += (long)elements_written;
            return (bytes_sent);
        }
    }

    while (fclose(stream) != 0)
        my_error(ERR_FILE_STREAM_STUCK);
    return (bytes_sent);
}

unsigned int swap_flash(unsigned int interval)
{
    u64 block;
    u32 words[2];
    u16 halfwords[4];
    u8 block_buf[8];
    unsigned int mask;
    register size_t i, j;

    if (interval != 0) { /* callee-defined endian swap on a fixed interval */
        mask = interval - 1; /* e.g. (interval = 4) for a 32-bit swap */
        goto swap_memory;
    }

/*
 * if (interval == 0) then the memory swapping, if any at all, is done based
 * on automatic detection of the current byte order the flash RAM is in.
 */
    i = 0x0000;
    do {
        block = read64(&flash_RAM[i + 0x0020]);
        if (block != 0 && block != ~0)
            break;
        i += FILE_SIZE;
    } while (i < FLASH_SIZE);
    if (i >= FLASH_SIZE) { /* overflow from searching an empty file */
        mask = get_client_swap_mask();
        fprintf(stdout, "Detected %s data.", "completely blank game");
        goto swap_memory;
    }

    words[0] = (u32)(block & 0x00000000FFFFFFFFul);
    words[1] = (u32)(block >> 32);
    switch (words[0]) {
    case /* 'ZELD' */0x5A454C44:
        mask = 0;
        fprintf(stdout, "Detected %s data.", "hardware-accurate");
        break;
    case /* 'EZDL' */0x455A444C:
        mask = 1;
        fprintf(stdout, "Detected %s data.", "16-bit byte-swapped");
        break;
    case /* 'LDZE' */0x4C445A45:
        mask = 2;
        fprintf(stdout, "Detected %s data.", "32-bit halfword-swapped");
        break;
    case /* 'DLEZ' */0x444C455A:
        mask = 3;
        fprintf(stdout, "Detected %s data.", "32-bit byte-swapped");
        break;
    default:
        switch (words[1]) {
        case /* 'ZELD' */0x5A454C44:
            mask = 4;
            fprintf(stdout, "Detected %s data.", "64-bit word-swapped");
            break;
        case /* 'DLEZ' */0x444C455A:
            mask = 7;
            fprintf(stdout, "Detected %s data.", "64-bit byte-swapped");
            break;
        default:
            fprintf(stderr, "Flash formatting damaged or unsupported.\n");
            return (mask = ~0u);
        }
    }

swap_memory:
    switch (mask) {
    case 0:
        break; /* Swapping big-endian into big-endian is a null operation. */
    case 1:
        for (i = 0; i < FLASH_SIZE; i += 2) {
            for (j = 0; j < 2; j++)
                block_buf[j] = read8(&flash_RAM[i + j]);
            write8(&flash_RAM[i + 0], block_buf[0 ^ 1]);
            write8(&flash_RAM[i + 1], block_buf[1 ^ 1]);
        }
        break;
    case 2:
        for (i = 0; i < FLASH_SIZE; i += 4) {
            for (j = 0; j < 2; j++)
                halfwords[j] = read16(&flash_RAM[i + 2*j]);
            write16(&flash_RAM[i + 0], halfwords[0 ^ 2/2]);
            write16(&flash_RAM[i + 2], halfwords[1 ^ 2/2]);
        }
        break;
    case 3:
        for (i = 0; i < FLASH_SIZE; i += 4) {
            for (j = 0; j < 4; j++)
                block_buf[j] = read8(&flash_RAM[i + j]);
            write8(&flash_RAM[i + 0], block_buf[0 ^ 3]);
            write8(&flash_RAM[i + 1], block_buf[1 ^ 3]);
            write8(&flash_RAM[i + 2], block_buf[2 ^ 3]);
            write8(&flash_RAM[i + 3], block_buf[3 ^ 3]);
        }
        break;
    case 4:
        for (i = 0; i < FLASH_SIZE; i += 8) {
            for (j = 0; j < 2; j++)
                words[j] = read32(&flash_RAM[i + 4*j]);
            write32(&flash_RAM[i + 0], words[0 ^ 4/4]);
            write32(&flash_RAM[i + 4], words[1 ^ 4/4]);
        }
        break;
    case 7:
        for (i = 0; i < FLASH_SIZE; i += 8) {
            for (j = 0; j < 8; j++)
                block_buf[j] = read8(&flash_RAM[i + j]);
            write8(&flash_RAM[i + 0], block_buf[0 ^ 7]);
            write8(&flash_RAM[i + 1], block_buf[1 ^ 7]);
            write8(&flash_RAM[i + 2], block_buf[2 ^ 7]);
            write8(&flash_RAM[i + 3], block_buf[3 ^ 7]);
            write8(&flash_RAM[i + 4], block_buf[4 ^ 7]);
            write8(&flash_RAM[i + 5], block_buf[5 ^ 7]);
            write8(&flash_RAM[i + 6], block_buf[6 ^ 7]);
            write8(&flash_RAM[i + 7], block_buf[7 ^ 7]);
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
