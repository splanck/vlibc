#include "getopt.h"
#include "stdio.h"
#include "string.h"

/*
 * Parse command line options allowing long options with a single dash.
 * If a matching long option is not found, fall back to getopt() for
 * short option processing.
 */
int getopt_long_only(int argc, char * const argv[], const char *optstring,
                     const struct option *longopts, int *longindex)
{
    if (optind >= argc)
        return -1;

    char *arg = argv[optind];
    if (arg[0] != '-' || arg[1] == '\0')
        return getopt(argc, argv, optstring);

    /* handle the --foo case */
    if (arg[1] == '-')
        arg += 2;
    else
        arg += 1;

    const char *eq = strchr(arg, '=');
    size_t len = eq ? (size_t)(eq - arg) : strlen(arg);

    for (int i = 0; longopts && longopts[i].name; i++) {
        if (strncmp(arg, longopts[i].name, len) == 0 &&
            longopts[i].name[len] == '\0') {
            if (longindex)
                *longindex = i;

            if (longopts[i].has_arg == required_argument) {
                if (eq)
                    optarg = (char *)(eq + 1);
                else if (optind + 1 < argc)
                    optarg = argv[++optind];
                else {
                    if (opterr)
                        fprintf(stderr, "option '--%s' requires argument\n",
                                longopts[i].name);
                    optind++;
                    optopt = longopts[i].val;
                    return optstring && optstring[0] == ':' ? ':' : '?';
                }
            } else if (longopts[i].has_arg == optional_argument) {
                optarg = eq ? (char *)(eq + 1) : NULL;
            } else {
                optarg = NULL;
            }

            optind++;
            if (longopts[i].flag) {
                *longopts[i].flag = longopts[i].val;
                return 0;
            }
            return longopts[i].val;
        }
    }

    /* no matching long option - treat as short */
    return getopt(argc, argv, optstring);
}
