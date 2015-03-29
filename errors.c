#ifdef _WIN32
#include <windows.h>
#else
#include <stdio.h>
#endif
#include "errors.h"

static const char errors[ERR_UNKNOWN + 1][32] = {
    "No current error at this time." ,
    "No flash memory location given.",

    "Unable to contact data storage.",
    "Failed to close data steam."    ,

    "File system flash read failed." ,
    "File system flash write failed.",

    "Flash read access violation."   ,
    "Flash write access violation."  ,

    "Unimplemented error identifier.",
};

void my_error(const int key)
{
    int code;

    if (key < ERR_NONE || key > ERR_UNKNOWN)
        code = ERR_UNKNOWN;
    else
        code = key;

#ifdef _WIN32
    MessageBoxA(NULL, errors[code], NULL, MB_ICONERROR);
#else
    fputs(errors[code], stderr);
    fputc('\n', stderr);
    fgetc(stdin); /* pausing for better notification */
#endif
    return;
}
