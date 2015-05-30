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
        size_t elements_written;

        elements_written = fwrite(&flash_RAM[bytes_sent], 8, 1, stream);
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
