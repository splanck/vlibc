/*
 * BSD 2-Clause License
 *
 * Purpose: Declarations for option parsing helpers.
 */
#ifndef GETOPT_H
#define GETOPT_H

extern char *optarg;
extern int optind;
extern int opterr;
extern int optopt;

/* long option handling */
struct option {
    const char *name;
    int has_arg;
    int *flag;
    int val;
};

#define no_argument        0
#define required_argument  1
#define optional_argument  2

int getopt(int argc, char * const argv[], const char *optstring);
int getopt_long(int argc, char * const argv[], const char *optstring,
               const struct option *longopts, int *longindex);
int getopt_long_only(int argc, char * const argv[], const char *optstring,
                     const struct option *longopts, int *longindex);

/* parse comma separated suboptions */
int getsubopt(char **optionp, char * const *tokens, char **valuep);

#endif /* GETOPT_H */
