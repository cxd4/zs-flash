#ifndef _ERRORS_H_
#define _ERRORS_H_

enum {
    ERR_NONE,

    ERR_FLASHRAM_LOCATION_UNKNOWN,
    ERR_MEMORY_FORMAT_UNKNOWN,

    ERR_FILE_STREAM_NO_LINK,
    ERR_FILE_STREAM_STUCK,

    ERR_DISK_READ_FAILURE,
    ERR_DISK_WRITE_FAILURE,

    ERR_RD_FLASH_ACCESS_VIOLATION,
    ERR_WR_FLASH_ACCESS_VIOLATION,

    ERR_OPTION_INVALID,
    ERR_OPTION_NOT_IMPLEMENTED,

    ERR_INTEGER_COUNT_INSUFFICIENT,
    ERR_INTEGER_TOO_LARGE,

    ERR_SIGNED_UNDERFLOW,
    ERR_SIGNED_OVERFLOW,

    ERR_OUT_OF_MEMORY,
    ERR_UNKNOWN
};

extern void my_error(const int key);

#endif
