// errors.h
// Nathan Corcoran, 2023
//

#if !defined(__ERRORS_H__)
#define __ERRORS_H__

#define ERR_SUCCESS_CODE 0
#define ERR_SUCCESS_MSG "%s: finished SUCCESSFULLY: %d\n"

#define ERR_USAGE_CODE 1
#define ERR_USAGE_MSG "Usage: %s [-alpha | -len | -longest] [-i c] letters [dictionary]: %d\n"

#define ERR_FILE_CODE 2
#define ERR_FILE_MSG "%s: dictionary file could not be opened: %d\n"

#define ERR_LENGTH_CODE 3
#define ERR_LENGTH_MSG "%s: must supply at least 3 letters: %d\n"

#define ERR_ALNUM_CODE 4
#define ERR_ALNUM_MSG "%s: letters supplied must be alphabetical: %d\n"

#define ERR_NOTHING_CODE 5
#define ERR_NOTHING_MSG "%s: no words found :^(: %d\n"

#define ERR_OOM_CODE ENOMEM
#define ERR_OOM_MSG "%s: could not allocate crucial memory: %d\n"

/**
 * Prints an error message and exits the program.
 *
 * @param err_code Code to exit with.
 * @param err_msg Message to print.
 * @param err_line Line number to print.
 */
static void
err_and_exit(
        int      err_code, 
        char    *err_msg,
        int      err_line
) {
    extern char *__progname;

    fprintf(stderr, err_msg, __progname, err_line);
    exit(err_code);
}

#endif /* __ERRORS_H__ */
