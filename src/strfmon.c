/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms, with or without modification, are permitted provided that the copyright notice and this permission notice appear in all copies. This software is provided "as is" without warranty.
 *
 * Purpose: Implements a minimal strfmon for vlibc. Provides basic POSIX compliant monetary formatting for ASCII locales.
 */

#include "monetary.h"
#include "string.h"
#include "stdio.h"
#include "errno.h"
#include <stdarg.h>

ssize_t strfmon(char *s, size_t max, const char *format, ...)
{
    if (!s || !format || max == 0) {
        errno = EINVAL;
        return -1;
    }

    va_list ap;
    va_start(ap, format);
    size_t pos = 0;

    for (const char *p = format; *p; ++p) {
        if (*p != '%') {
            if (pos + 1 >= max) {
                errno = E2BIG;
                va_end(ap);
                return -1;
            }
            s[pos++] = *p;
            continue;
        }
        ++p;
        if (*p == '%') {
            if (pos + 1 >= max) {
                errno = E2BIG;
                va_end(ap);
                return -1;
            }
            s[pos++] = '%';
            continue;
        }

        int left_align = 0;
        int width = 0;
        int prec = 2;
        /* Skip flags we don't implement */
        for (;; ++p) {
            if (*p == '-') {
                left_align = 1;
            } else if (*p == '+' || *p == '(' || *p == '!' || *p == '^' || *p == '=') {
                /* ignore */
            } else {
                break;
            }
        }
        /* Parse width */
        while (*p >= '0' && *p <= '9') {
            width = width * 10 + (*p - '0');
            ++p;
        }
        if (*p == '.') {
            ++p;
            prec = 0;
            while (*p >= '0' && *p <= '9') {
                prec = prec * 10 + (*p - '0');
                ++p;
            }
        }
        char conv = *p;
        if (conv != 'n' && conv != 'i') {
            errno = EINVAL;
            va_end(ap);
            return -1;
        }
        double val = va_arg(ap, double);
        int neg = val < 0.0;
        if (neg)
            val = -val;

        char numbuf[64];
        if (snprintf(numbuf, sizeof(numbuf), "%.*f", prec, val) >= (int)sizeof(numbuf)) {
            errno = E2BIG;
            va_end(ap);
            return -1;
        }

        char curbuf[80];
        if (neg)
            snprintf(curbuf, sizeof(curbuf), "-$%s", numbuf);
        else
            snprintf(curbuf, sizeof(curbuf), "$%s", numbuf);

        size_t len = strlen(curbuf);
        size_t pad = 0;
        if (width > 0 && (size_t)width > len)
            pad = (size_t)width - len;

        if (!left_align) {
            if (pos + pad + len >= max) {
                errno = E2BIG;
                va_end(ap);
                return -1;
            }
            for (size_t i = 0; i < pad; ++i)
                s[pos++] = ' ';
            memcpy(s + pos, curbuf, len);
            pos += len;
        } else {
            if (pos + pad + len >= max) {
                errno = E2BIG;
                va_end(ap);
                return -1;
            }
            memcpy(s + pos, curbuf, len);
            pos += len;
            for (size_t i = 0; i < pad; ++i)
                s[pos++] = ' ';
        }
    }

    if (pos >= max) {
        errno = E2BIG;
        va_end(ap);
        return -1;
    }
    s[pos] = '\0';
    va_end(ap);
    return (ssize_t)pos;
}

