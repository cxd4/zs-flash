#ifndef _ZS_DATA_H_
#define _ZS_DATA_H_

/*
 * "ZELDA3" is the magic number.  If it's stored at 0x0024 into the section
 * in question, then said section stores game data for a saved file.
 *
 * The return value is exactly the same as with memcmp() or strcmp().
 */
extern int magic_number_test(unsigned int section_ID);

/*
 * Safety checking of flash reads and writes by this game is maintained by
 * means of a normal, additive 16-bit checksum.  The relevant checksum will
 * be stored in only one of two places, depending on whether the version of
 * the ROM supports saving through owl statues or not (non-Japanese ROMs).
 *
 * The correct and up-to-date checksum value is returned.
 */
extern u16 fix_checksum(unsigned int section_ID);

/*
 * Execute a command-line option for modifying saved data.
 * optv[0] is the switch (e.g., "-x") for what is modified (an op-code).
 *
 * The return value of this function is optc (or argc), which represents the
 * count of all arguments to the option--i.e., the number of arguments until
 * the next argv[] that begins with "-" or the end of the command buffer.
 */
extern int opt_execute(char ** optv);

#endif
