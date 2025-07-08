/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms, with
 * or without modification, are permitted provided that the copyright notice and
 * this permission notice appear in all copies. This software is provided "as is" without warranty.
 *
 * Purpose: Implements abs, labs, llabs, div, ldiv and lldiv for vlibc.
 */

#include "stdlib.h"
#include <limits.h>

/* Return the absolute value of a signed int. */
int abs(int j)
{
    if (j < 0) {
        if (j == INT_MIN)
            return j;
        return -j;
    }
    return j;
}

/* Return the absolute value of a signed long. */
long labs(long j)
{
    if (j < 0) {
        if (j == LONG_MIN)
            return j;
        return -j;
    }
    return j;
}

/* Return the absolute value of a signed long long. */
long long llabs(long long j)
{
    if (j < 0) {
        if (j == LLONG_MIN)
            return j;
        return -j;
    }
    return j;
}

/* Compute quotient and remainder of numer divided by denom. */
div_t div(int numer, int denom)
{
    div_t r;
    r.quot = numer / denom;
    r.rem  = numer % denom;
    return r;
}

/* Long variant of div(). Returns quotient and remainder. */
ldiv_t ldiv(long numer, long denom)
{
    ldiv_t r;
    r.quot = numer / denom;
    r.rem  = numer % denom;
    return r;
}

/* Long long variant of div(). Returns quotient and remainder. */
lldiv_t lldiv(long long numer, long long denom)
{
    lldiv_t r;
    r.quot = numer / denom;
    r.rem  = numer % denom;
    return r;
}

