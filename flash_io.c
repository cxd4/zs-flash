#include "flash_io.h"
#include "errors.h"

unsigned char flash_RAM[FLASH_SIZE];

unsigned char read8(const unsigned char * addr)
{
    unsigned char octet;

    if (addr < &flash_RAM[FLASH_MIN_ADDR] || addr > &flash_RAM[FLASH_MAX_ADDR])
    {
        my_error(ERR_RD_FLASH_ACCESS_VIOLATION);
        return 0x00u; /* N64 RCP reads 0 from un-mapped DRAM. */
    }
    octet = *(addr) & 0xFFu;
    return (octet);
}

unsigned short read16(const unsigned char * addr)
{
    unsigned short halfword; /* MIPS and from the game's point of view */

    halfword  = read8(addr + 0) << 8;
    halfword |= read8(addr + 1) << 0;
    return (halfword & 0x000000000000FFFFu);
}

unsigned long read32(const unsigned char * addr)
{
    u32 word; /* MIPS and from the game's point of view */

    word  = read16(addr + 0) << 16;
    word |= read16(addr + 2) <<  0;
    return (word & 0x00000000FFFFFFFFu);
}

void * read64(const unsigned char * addr)
{
    static u32 doubleword[2];

    doubleword[0] = read32(addr + 0);
    doubleword[1] = read32(addr + 4);
    return &(doubleword[0]);
}

void write8(unsigned char * dst, const unsigned char * src)
{
    if (dst < &flash_RAM[FLASH_MIN_ADDR] || dst > &flash_RAM[FLASH_MAX_ADDR])
    {
        my_error(ERR_WR_FLASH_ACCESS_VIOLATION);
        return; /* N64 RCP writes nothing to un-mapped memory. */
    }
    *(dst) = read8(src);
    return;
}

void write16(unsigned char *  dst, const unsigned char * src)
{
    write8(dst + 0, src + 0);
    write8(dst + 1, src + 1);
    return; 
}

void write32(unsigned char * dst, const unsigned char * src)
{
    write16(dst + 0, src + 0);
    write16(dst + 2, src + 2);
    return;
}

void write64(unsigned char * dst, const unsigned char * src)
{
    write32(dst + 0, src + 0);
    write32(dst + 4, src + 4);
    return;
}
