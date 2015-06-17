#ifndef _ZS_DATA_H_
#define _ZS_DATA_H_

#define BYTES_IN_MAGIC_NUMBER           6

typedef int(*p_opt)(int optc, char ** argv);
extern p_opt * opt_table;

extern unsigned int swap_mask;

/*
 * Set to &flash_RAM[0x0000] for File 1 and &flash_RAM[0x4000] for File 2.
 * It basically is a reference to &flash_RAM[section_ID << 13].
 */
extern u8 * file;


extern int player_mask(int optc, char ** optv);
extern int zelda_time(int optc, char ** optv);
extern int change_zelda_time(int optc, char ** optv);
extern int totalday(int optc, char ** optv);
extern int player_character(int optc, char ** optv);
extern int bell_flag(int optc, char ** optv);
extern int life_energy_points(int optc, char ** optv); /* max_life, now_life */
extern int magic_points(int optc, char ** optv); /* magic_max, magic_now */
extern int lupy_count(int optc, char ** optv);
extern int long_sword_hp(int optc, char ** optv);
extern int collect_register(int optc, char ** optv);
extern int key_compass_map(int optc, char ** optv);
extern int key_register(int optc, char ** optv);
extern int orange_fairy(int optc, char ** optv);

extern void init_options(void);
extern int reserved(int optc, char ** optv);
extern int zs_endian_swap_mask(int optc, char ** optv);
extern int zs_file_pointer(int optc, char ** optv);

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

extern int show8 (const char * name, size_t offset);
extern int show16(const char * name, size_t offset);
extern int show32(const char * name, size_t offset);

extern int send8 (size_t offset, unsigned long input); /* zero-extended */
extern int send16(size_t offset, unsigned long input);
extern int send32(size_t offset, unsigned long input);

extern int sendx8 (size_t offset, signed long input); /* sign-extended */
extern int sendx16(size_t offset, signed long input);
extern int sendx32(size_t offset, signed long input);

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
