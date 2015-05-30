#ifndef _ZS_DATA_H_
#define _ZS_DATA_H_

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
