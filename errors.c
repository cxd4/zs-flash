#include <stdio.h>
#include "errors.h"

static const char errors[ERR_UNKNOWN + 1][32] = {
    "No current error at this time." ,

    "No flash memory location given.",
    "Corrupt or unknown byte order." ,

    "Unable to contact data storage.",
    "Failed to close data steam."    ,

    "File system flash read failed." ,
    "File system flash write failed.",

    "Flash read access violation."   ,
    "Flash write access violation."  ,

    "Invalid option switch prefix."  ,
    "Unimplemented option switch."   ,

    "Too few arguments to this flag.",
    "Value too large for this input.",

    "Signed underflow of user input.",
    "Signed overflow of user input." ,

    "Unimplemented error identifier.",
};

void my_error(const int key)
{
    int code;

    if (key < ERR_NONE || key > ERR_UNKNOWN)
        code = ERR_UNKNOWN;
    else
        code = key;

    fputs(errors[code], stderr);
    fputc('\n', stderr);
    fgetc(stdin); /* pausing for better notification */
    return;
}
