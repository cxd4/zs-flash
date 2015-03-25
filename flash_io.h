#ifndef _FLASH_IO_H_
#define _FLASH_IO_H_

#define FLASH_MIN_ADDR          0x000000
#define FLASH_MAX_ADDR          0x01FFFF
#define FILE_MIN_ADDR           0x000000
#define FILE_MAX_ADDR           0x001FFF

#if ((FLASH_MIN_ADDR) % 8 != 0 || (FLASH_MAX_ADDR) % 8 != 7)
#error Unaligned RCP address range for flash memory.
#endif

#if ((FILE_MIN_ADDR) % 8 != 0 || (FILE_MAX_ADDR) % 8 != 7)
#error Unaligned flash memory range for save file segmentation.
#endif

#define FLASH_SIZE              ((FLASH_MAX_ADDR) - (FLASH_MIN_ADDR) + 1)
#define FILE_SIZE               ((FILE_MAX_ADDR) - (FILE_MIN_ADDR) + 1)

#define NUMBER_OF_DATA_FILES    ((FLASH_SIZE) / (FILE_SIZE))

extern unsigned char flash_RAM[FLASH_SIZE];

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
 * 64-bit type on the hardware (presuming LP64 ABI implementations of C that
 * make some effort to match types to register sizes), so there is not
 * really a mandatory use for C99 extensions like `long long` or `int64_t`.
 */
#if (0xFFFFFFFFl == 0xFFFFFFFFul)
typedef signed long             s64;
typedef unsigned long           u64;
#endif

/*
 * just in case signedness does not matter at some point, for readability
 */
typedef char                    i8;
typedef s16                     i16;
typedef s32                     i32;

extern unsigned char  read8 (const unsigned char * addr);
extern unsigned short read16(const unsigned char * addr);
extern unsigned long  read32(const unsigned char * addr);
extern void*          read64(const unsigned char * addr);

extern void write8 (unsigned char * dst, const unsigned char * src);
extern void write16(unsigned char * dst, const unsigned char * src);
extern void write32(unsigned char * dst, const unsigned char * src);
extern void write64(unsigned char * dst, const unsigned char * src);

#endif
