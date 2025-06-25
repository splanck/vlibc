/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms,
 * with or without modification, are permitted provided that the copyright
 * notice and this permission notice appear in all copies. This software is
 * provided "as is" without warranty.
 *
 * Purpose: Implements the err/warn helpers for vlibc.
 */

#include "err.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "errno.h"

static void vwarn_internal(const char *fmt, va_list ap, int use_errno)
{
    if (fmt && *fmt)
        vfprintf(stderr, fmt, ap);
    if (use_errno) {
        if (fmt && *fmt)
            fprintf(stderr, ": %s", strerror(errno));
        else
            fprintf(stderr, "%s", strerror(errno));
    }
    fprintf(stderr, "\n");
}

void vwarn(const char *fmt, va_list ap)
{
    vwarn_internal(fmt, ap, 1);
}

void warn(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vwarn(fmt, ap);
    va_end(ap);
}

void vwarnx(const char *fmt, va_list ap)
{
    vwarn_internal(fmt, ap, 0);
}

void warnx(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vwarnx(fmt, ap);
    va_end(ap);
}

static void verr_internal(int status, const char *fmt, va_list ap, int use_errno)
{
    vwarn_internal(fmt, ap, use_errno);
    exit(status);
}

void verr(int status, const char *fmt, va_list ap)
{
    verr_internal(status, fmt, ap, 1);
}

void err(int status, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    verr(status, fmt, ap);
    va_end(ap);
}

void verrx(int status, const char *fmt, va_list ap)
{
    verr_internal(status, fmt, ap, 0);
}

void errx(int status, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    verrx(status, fmt, ap);
    va_end(ap);
}
