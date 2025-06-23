#ifndef REGEX_H
#define REGEX_H

#include <stddef.h>

/* simple regular expression interface */

typedef struct {
    void *impl;        /* internal compiled form */
    size_t re_nsub;    /* number of capture groups (unused) */
} regex_t;

typedef struct {
    int rm_so; /* start offset */
    int rm_eo; /* end offset */
} regmatch_t;

/* compile pattern into preg */
int regcomp(regex_t *preg, const char *pattern, int cflags);

/* execute regex on string */
int regexec(const regex_t *preg, const char *string,
            size_t nmatch, regmatch_t pmatch[], int eflags);

/* free compiled pattern */
void regfree(regex_t *preg);

/* error code when no match was found */
#define REG_NOMATCH 1

#endif /* REGEX_H */
