#include "flash_io.h"
#include "errors.h"

unsigned char flash_RAM[FLASH_SIZE];

u8 read8(const unsigned char * addr)
{
    u8 octet;

    if (addr < &flash_RAM[FLASH_MIN_ADDR] || addr > &flash_RAM[FLASH_MAX_ADDR])
    {
        my_error(ERR_RD_FLASH_ACCESS_VIOLATION);
        return 0x00u; /* N64 RCP reads 0 from un-mapped DRAM. */
    }
    octet = *(addr) & 0xFFu;
    return (octet);
}

u16 read16(const unsigned char * addr)
{
    u16 halfword; /* MIPS and from the game's point of view */

    halfword  = read8(addr + 0) << 8;
    halfword |= read8(addr + 1) << 0;
    return (halfword & 0x000000000000FFFFu);
}

u32 read32(const unsigned char * addr)
{
    u32 word; /* MIPS and from the game's point of view */

    word  = read16(addr + 0) << 16;
    word |= read16(addr + 2) <<  0;
    return (word & 0x00000000FFFFFFFFu);
}

u64 read64(const unsigned char * addr)
{
    u64 doubleword;

    doubleword  = (u64)read32(addr + 0) << 32;
    doubleword |= (u64)read32(addr + 4) <<  0;
    return (doubleword);
}

void write8(unsigned char * dst, const u8 src)
{
    if (dst < &flash_RAM[FLASH_MIN_ADDR] || dst > &flash_RAM[FLASH_MAX_ADDR])
    {
        my_error(ERR_WR_FLASH_ACCESS_VIOLATION);
        return; /* N64 RCP writes nothing to un-mapped memory. */
    }
    *(dst) = (unsigned char)src; /* maybe sizeof(u8) != sizeof(unsigned char) */
    return;
}

void write16(unsigned char * dst, const u16 src)
{
    const u16 src_mask_hi = (src & 0xFF00u) >>  8;
    const u16 src_mask_lo = (src & 0x00FFu) >>  0;

    write8(dst + 0, (u8)src_mask_hi);
    write8(dst + 1, (u8)src_mask_lo);
    return;
}

void write32(unsigned char * dst, const u32 src)
{
    const u32 src_mask_hi = (src & 0xFFFF0000ul) >> 16;
    const u32 src_mask_lo = (src & 0x0000FFFFul) >>  0;

    write16(dst + 0, (u16)src_mask_hi);
    write16(dst + 2, (u16)src_mask_lo);
    return;
}

void write64(unsigned char * dst, const u64 src)
{
    const u32 src_lo      = (u32)(src & 0x00000000FFFFFFFFul);
    const u64 src_mask_hi = (src ^ src_lo) >> 32;

    write32(dst + 0, (u32)src_mask_hi);
    write32(dst + 4, src_lo);
    return;
}
