#include <memory.h>
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
    if (address < min_addr || address > max_addr)
    {
        my_error(ERR_RD_FLASH_ACCESS_VIOLATION);
        return 0x00u; /* N64 RCP reads 0 from un-mapped DRAM. */
    }
    memcpy(&octet, address, sizeof(i8));
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
    return (word & 0x00000000FFFFFFFFu);
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
    if (dst < min_addr || dst > max_addr)
    {
        my_error(ERR_WR_FLASH_ACCESS_VIOLATION);
        return; /* N64 RCP writes nothing to un-mapped memory. */
    }
    memcpy(dst, &src, sizeof(i8));
    return;
}

void write16(void * dst, const u16 src)
{
    u8 * addr;
    const u16 src_mask_hi = (src & 0xFF00u) >>  8;
    const u16 src_mask_lo = (src & 0x00FFu) >>  0;
    const u8 src_hi = (u8)(src_mask_hi & 0x00FFu);
    const u8 src_lo = (u8)(src_mask_lo & 0x00FFu);

    addr = (u8 *)dst;
    write8(addr + 0, src_hi);
    write8(addr + 1, src_lo);
    return;
}

void write32(void * dst, const u32 src)
{
    u8 * addr;
    const u32 src_mask_hi = (src & 0xFFFF0000ul) >> 16;
    const u32 src_mask_lo = (src & 0x0000FFFFul) >>  0;
    const u16 src_hi = (u16)(src_mask_hi & 0x0000FFFFu);
    const u16 src_lo = (u16)(src_mask_lo & 0x0000FFFFu);

    addr = (u8 *)dst;
    write16(addr + 0, src_hi);
    write16(addr + 2, src_lo);
    return;
}

void write64(void * dst, const u64 src)
{
    u8 * addr;
    const u64 src_mask_lo = (src & 0x00000000FFFFFFFFul) >>  0;
    const u64 src_mask_hi = (src ^ src_mask_lo         ) >> 32;
    const u32 src_hi      = (u32)(src_mask_hi & 0x00000000FFFFFFFFul);
    const u32 src_lo      = (u32)(src_mask_lo & 0x00000000FFFFFFFFul);

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
    if (stream == NULL)
    {
        my_error(ERR_FILE_STREAM_NO_LINK);
        return (bytes_read = 0);
    }

    for (bytes_read = 0; bytes_read < FLASH_SIZE; bytes_read += BLOCK_SIZE)
    {
        u8 block_buf[8];
        u64 block;
        size_t elements_read;

        elements_read = fread(&block_buf[0], 8, 1, stream);
        if (elements_read != 1)
        {
            my_error(ERR_DISK_READ_FAILURE);
            return (bytes_read);
        }

        block =
            ((u64)block_buf[0] << 56)
          | ((u64)block_buf[1] << 48)
          | ((u64)block_buf[2] << 40)
          | ((u64)block_buf[3] << 32)
          | ((u64)block_buf[4] << 24)
          | ((u64)block_buf[5] << 16)
          | ((u64)block_buf[6] <<  8)
          | ((u64)block_buf[7] <<  0)
        ;
        write64(&flash_RAM[bytes_read], block);
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
    if (stream == NULL)
    {
        my_error(ERR_FILE_STREAM_NO_LINK);
        return (bytes_sent = 0);
    }

    for (bytes_sent = 0; bytes_sent < FLASH_SIZE; bytes_sent += BLOCK_SIZE)
    {
        u8 block_buf[8];
        u64 block;
        size_t elements_written;

        block = read64(&flash_RAM[bytes_sent]);
        block_buf[0] = (u8)((block >> 56) & 0xFF);
        block_buf[1] = (u8)((block >> 48) & 0xFF);
        block_buf[2] = (u8)((block >> 40) & 0xFF);
        block_buf[3] = (u8)((block >> 32) & 0xFF);
        block_buf[4] = (u8)((block >> 24) & 0xFF);
        block_buf[5] = (u8)((block >> 16) & 0xFF);
        block_buf[6] = (u8)((block >>  8) & 0xFF);
        block_buf[7] = (u8)((block >>  0) & 0xFF);

        elements_written = fwrite(&block_buf[0], 8, 1, stream);
        if (elements_written != 1)
        {
            my_error(ERR_DISK_WRITE_FAILURE);
            return (bytes_sent);
        }
    }

    while (fclose(stream) != 0)
        my_error(ERR_FILE_STREAM_STUCK);
    return (bytes_sent);
}

unsigned int swap_flash(void)
{
    u64 block;
    u32 words[2];
    u16 halfwords[4];
    u8 block_buf[8];
    unsigned int mask;
    register size_t i, j;

    for (i = 0x0000; i < FLASH_SIZE; i += FILE_SIZE)
    {
        block = read64(&flash_RAM[i + 0x0020]);
        if (block != 0 && block != ~0)
            break;
    }
    words[0] = (u32)(block & 0x00000000FFFFFFFFul);
    words[1] = (u32)(block >> 32);
    switch (words[0])
    {
    case 'ZELD':
        mask = 0;
        fprintf(stdout, "Detected %s data.", "hardware-accurate");
        break;
    case 'EZDL':
        mask = 1;
        fprintf(stdout, "Detected %s data.", "16-bit byte-swapped");
        break;
    case 'LDZE':
        mask = 2;
        fprintf(stdout, "Detected %s data.", "32-bit halfword-swapped");
        break;
    case 'DLEZ':
        mask = 3;
        fprintf(stdout, "Detected %s data.", "32-bit byte-swapped");
        break;
    default:
        switch (words[1])
        {
        case 'ZELD':
            mask = 4;
            fprintf(stdout, "Detected %s data.", "64-bit word-swapped");
            break;
        case 'DLEZ':
            mask = 7;
            fprintf(stdout, "Detected %s data.", "64-bit byte-swapped");
            break;
        default:
            fprintf(stderr, "Flash formatting damaged or unsupported.");
            return (mask = 0);
        }
    }

    switch (mask)
    {
    case 0:
        break; /* Swapping big-endian into big-endian is a null operation. */
    case 1:
        for (i = 0; i < FLASH_SIZE; i += 2)
        {
            for (j = 0; j < 2; j++)
                block_buf[j] = read8(&flash_RAM[i + j]);
            write8(&flash_RAM[i + 0], block_buf[0 ^ 1]);
            write8(&flash_RAM[i + 1], block_buf[1 ^ 1]);
        }
        break;
    case 2:
        for (i = 0; i < FLASH_SIZE; i += 4)
        {
            for (j = 0; j < 2; j++)
                halfwords[j] = read16(&flash_RAM[i + 2*j]);
            write16(&flash_RAM[i + 0], halfwords[0 ^ 2/2]);
            write16(&flash_RAM[i + 2], halfwords[1 ^ 2/2]);
        }
        break;
    case 3:
        for (i = 0; i < FLASH_SIZE; i += 4)
        {
            for (j = 0; j < 4; j++)
                block_buf[j] = read8(&flash_RAM[i + j]);
            write8(&flash_RAM[i + 0], block_buf[0 ^ 3]);
            write8(&flash_RAM[i + 1], block_buf[1 ^ 3]);
            write8(&flash_RAM[i + 2], block_buf[2 ^ 3]);
            write8(&flash_RAM[i + 3], block_buf[3 ^ 3]);
        }
        break;
    case 4:
        for (i = 0; i < FLASH_SIZE; i += 8)
        {
            for (j = 0; j < 2; j++)
                words[j] = read32(&flash_RAM[i + 4*j]);
            write32(&flash_RAM[i + 0], words[0 ^ 4/4]);
            write32(&flash_RAM[i + 4], words[1 ^ 4/4]);
        }
        break;
    case 7:
        for (i = 0; i < FLASH_SIZE; i += 8)
        {
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
