#include "getopt.h"
#include "stdio.h"
#include "string.h"

char *optarg = NULL;
int optind = 1;
int opterr = 1;
int optopt = 0;

int getopt(int argc, char * const argv[], const char *optstring)
{
    static const char *next = NULL;

    if (!next || *next == '\0') {
        if (optind >= argc)
            return -1;
        const char *arg = argv[optind];
        if (arg[0] != '-' || arg[1] == '\0')
            return -1;
        if (strcmp(arg, "--") == 0) {
            optind++;
            return -1;
        }
        next = arg + 1;
        optind++;
    }

    char c = *next++;
    const char *pos = strchr(optstring, c);
    if (!pos) {
        optopt = c;
        if (opterr && optstring[0] != ':')
            fprintf(stderr, "unknown option -%c\n", c);
        return '?';
    }

    if (pos[1] == ':') {
        if (*next != '\0') {
            optarg = (char *)next;
            next = NULL;
        } else if (optind < argc) {
            optarg = argv[optind++];
            next = NULL;
        } else {
            optopt = c;
            if (opterr && optstring[0] != ':')
                fprintf(stderr, "option -%c requires argument\n", c);
            return optstring[0] == ':' ? ':' : '?';
        }
    } else {
        optarg = NULL;
    }

    return c;
}
