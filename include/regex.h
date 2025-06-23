#ifndef REGEX_H
#define REGEX_H

#include <stddef.h>

/* simple regular expression interface */

typedef struct {
    void *impl;        /* internal compiled form */
    size_t re_nsub;    /* number of capture groups */
} regex_t;

typedef struct {
    int rm_so; /* start offset */
    int rm_eo; /* end offset */
} regmatch_t;

/* compile pattern into preg.
 * Parentheses create numbered capture groups that can be
 * referenced in the pattern via \1, \2, ...
 */
int regcomp(regex_t *preg, const char *pattern, int cflags);

/* execute regex on string. If pmatch is provided it will be
 * filled with offsets for each capture group.
 */
int regexec(const regex_t *preg, const char *string,
            size_t nmatch, regmatch_t pmatch[], int eflags);

/* free compiled pattern */
void regfree(regex_t *preg);

/* error code when no match was found */
#define REG_NOMATCH 1

#endif /* REGEX_H */
