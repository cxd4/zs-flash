#include <stdio.h>
#include "errors.h"
#include "flash_io.h"
#include "zs_data.h"

int opt_execute(char ** optv)
{
    int optc;

    optc = 1;
    if (optv[0][0] != '-')
    {
        my_error(ERR_OPTION_INVALID);
        return (optc);
    }

    while (optv[optc] != NULL)
    {
        if (optv[optc][0] == '\0')
            break; /* We've reached the end of the command buffer. */
        if (optv[optc][0] == '-')
            break;
        ++optc;
    }

    switch (optv[0][1])
    {
    default:
        my_error(ERR_OPTION_NOT_IMPLEMENTED);
        break;
    }
    return (optc);
}
