#if !defined(GETOPT_H__)
#define GETOPT_H__

#define NULL_ARGUMENT     0
#define NO_ARGUMENT       0
#define REQUIRED_ARGUMENT 1
#define OPTIONAL_ARGUMENT 2

#define ARG_NULL 0
#define ARG_NONE 0
#define ARG_REQ  1
#define ARG_OPT  2

#include <string.h>
#include <wchar.h>

extern int optind;
extern int opterr;
extern int optopt;

struct option_a {
    const char* name;
    int has_arg;
    int *flag;
    int val;
};

extern char* optarg;
extern int getopt(int argc, char* const* argv, const char* optstring) throw();

#endif