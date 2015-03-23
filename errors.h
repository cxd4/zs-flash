#ifndef _ERRORS_H_
#define _ERRORS_H_

enum {
    ERR_NONE,
    ERR_FLASHRAM_LOCATION_UNKNOWN,
    ERR_RD_FLASH_ACCESS_VIOLATION,
    ERR_WR_FLASH_ACCESS_VIOLATION,

    ERR_UNKNOWN
};

extern void my_error(const int key);

#endif
