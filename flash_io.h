#ifndef _FLASH_IO_H_
#define _FLASH_IO_H_

#define FLASH_MIN_ADDR          0x000000
#define FLASH_MAX_ADDR          0x01FFFF
#define FILE_MIN_ADDR           0x000000
#define FILE_MAX_ADDR           0x001FFF

/*
 * RCP has only block-access (64-bit intervals) to the flash RAM.
 */
#define BLOCK_SIZE              8

#if ((FLASH_MIN_ADDR) % BLOCK_SIZE != 0 || (FLASH_MAX_ADDR) % BLOCK_SIZE != 7)
#error Unaligned RCP address range for flash memory.
#endif

#if ((FILE_MIN_ADDR) % BLOCK_SIZE != 0 || (FILE_MAX_ADDR) % BLOCK_SIZE != 7)
#error Unaligned flash memory range for save file segmentation.
#endif

#define FLASH_SIZE              ((FLASH_MAX_ADDR) - (FLASH_MIN_ADDR) + 1)
#define FILE_SIZE               ((FILE_MAX_ADDR) - (FILE_MIN_ADDR) + 1)

#define NUMBER_OF_DATA_FILES    ((FLASH_SIZE) / (FILE_SIZE))

#if defined(LITTLE_ENDIAN_64_FIXED)
#define ENDIAN_SWAP_BYTE    07
#elif defined(LITTLE_ENDIAN_32_FIXED)
#define ENDIAN_SWAP_BYTE    03
#elif defined(LITTLE_ENDIAN_16_FIXED)
#define ENDIAN_SWAP_BYTE    01
#else
#define ENDIAN_SWAP_BYTE    00
#endif

/*
 * GCC and the various compilers based on it (which also may define this)
 * have been known to bundle "stdint.h", which does not necessarily break
 * strict C89 compliance by itself, even though the existence of the header
 * is guaranteed only by C99 extensions or full support.
 */
#if defined(__GNUC__)
#include <stdint.h>
#endif

/*
 * smallest C data type that is greater than or equal to 8 bits
 *
 * After the 1989 ISO ratification, only `char` can possibly be 8-bit.
 */
typedef signed char             s8;
typedef unsigned char           u8;

/*
 * smallest C data type that is greater than or equal to 16 bits
 *
 * After the 1989 ISO ratification, only `short` and `char` are
 * possibilities.  (Very rarely should `char` be a 16-bit type, though.)
 * Obviously, `int` is also always greater than or equal to 16 bits,
 * but sizeof(int) >= sizeof(short), when `short` already had that guarantee.
 */
typedef signed short            s16;
typedef unsigned short          u16;

/*
 * smallest C data type that is greater than or equal to 32 bits
 *
 * `short` could be 32 bits long, but then we would lack 8- or 16-bit types.
 * It's better to avoid using `long` if we know that `int` already works.
 */
#if (0xFFFFFFFFL < 0xFFFFFFFFUL)
typedef signed long             s32;
typedef unsigned long           u32;
#else
typedef signed int              s32;
typedef unsigned int            u32;
#endif

/*
 * smallest C data type that is greater than or equal to 64 bits
 *
 * If `long` is not at least a 64-bit type, then there probably is no native
 * LP64 64-bit type on the hardware anyway, so any C99 extension to achieve
 * it would only be virtual, not direct.  However, the RCP's 64-bit access to
 * flash RAM does tempt the possible inclusion of even such a bypass.
 */
#if (0x00000000FFFFFFFFUL < ~0UL) && defined(__LP64__)
typedef signed long             s64;
typedef unsigned long           u64;
#elif defined(_STDINT_H)
typedef int64_t                 s64;
typedef uint64_t                u64;
#elif defined(MSC_VER_) /* Microsoft's own LLP64 in defense of their WINAPI */
typedef signed __int64          s64;
typedef unsigned __int64        u64;
#else /* fallback to assume C99 support if no physical 64-bit size exists */
typedef signed long long        s64;
typedef unsigned long long      u64;
#endif

/*
 * just in case signedness does not matter at some point, for readability
 */
typedef char                    i8;
typedef s16                     i16;
typedef s32                     i32;
typedef s64                     i64;

typedef union {
    u64 block;

    s64 doubleword;
    u32 words[2];
    u16 halfwords[4];
    u8 bytes[8];
} RCP_block;

extern u8 flash_RAM[FLASH_SIZE];

/*
 * reads from flash-native endian to client-native endian
 * (on x86, from MIPS big-endian to little-endian return slot)
 */
extern u8  read8 (const void * address);
extern u16 read16(const void * address);
extern u32 read32(const void * address);
extern u64 read64(const void * address);

/*
 * writes from client-native endian to flash-native endian
 * (on x86, from little-endian source to MIPS big-endian memory storage)
 */
extern void write8 (void * dst, const u8  src);
extern void write16(void * dst, const u16 src);
extern void write32(void * dst, const u32 src);
extern void write64(void * dst, const u64 src);

/*
 * memory management functions for the entire flash RAM
 * The only portable construct we have for flash I/O is C file system access.
 */
extern long load_flash(const char * filename);
extern long save_flash(const char * filename);
extern unsigned int swap_flash(unsigned int interval);

/*
 * When swap_flash() is called with 0 as the parameter, the memory is swapped
 * on an automatically detected interval that is based on the current byte
 * order found in the game data.  However, if there is no game data in the
 * entire flash memory, then automatic detection cannot occur based on this
 * condition alone.  We fall back to a new detection, get_client_swap_mask(),
 * which uses the current hardware's native byte order as the swap mask.
 */
extern unsigned int get_client_swap_mask(void);

#endif
