/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms, with or without modification, are permitted provided that the copyright notice and this permission notice appear in all copies. This software is provided "as is" without warranty.
 *
 * Purpose: Implements the wprintf functions for vlibc. Provides wrappers and helpers used by the standard library.
 *
 * Copyright (c) 2025
 */

#include "stdio.h"
#include "wchar.h"
#include "memory.h"
#include "io.h"
#include <stdarg.h>
#include <string.h>
#include <stdint.h>

static int wuint_to_base(unsigned long value, unsigned base, int upper,
                        wchar_t *buf, size_t size)
{
    const char *digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    char tmp[32];
    size_t i = 0;
    do {
        tmp[i++] = digits[value % base];
        value /= base;
    } while (value && i < sizeof(tmp));
    if (i > size)
        i = size;
    for (size_t j = 0; j < i; ++j)
        buf[j] = (wchar_t)tmp[i - j - 1];
    return (int)i;
}

static void wout_char(wchar_t *dst, size_t size, size_t *pos, wchar_t c)
{
    if (*pos + 1 < size)
        dst[*pos] = c;
    (*pos)++;
}

static void wout_str(wchar_t *dst, size_t size, size_t *pos,
                    const wchar_t *s, size_t len)
{
    for (size_t i = 0; i < len; i++)
        wout_char(dst, size, pos, s[i]);
}

static int vswprintf_impl(wchar_t *str, size_t size, const wchar_t *fmt, va_list ap)
{
    size_t pos = 0;
    for (const wchar_t *p = fmt; *p; ++p) {
        if (*p != L'%') {
            wout_char(str, size, &pos, *p);
            continue;
        }
        ++p;
        if (*p == L'%') {
            wout_char(str, size, &pos, L'%');
            continue;
        }

        int width = 0;
        int precision = -1;
        while (*p >= L'0' && *p <= L'9') {
            width = width * 10 + (*p - L'0');
            ++p;
        }
        if (*p == L'.') {
            ++p;
            precision = 0;
            while (*p >= L'0' && *p <= L'9') {
                precision = precision * 10 + (*p - L'0');
                ++p;
            }
        }

        wchar_t spec = *p;
        wchar_t buf[64];
        int len = 0;
        const wchar_t *prefix = L"";
        int prefix_len = 0;
        int sign = 0;

        switch (spec) {
        case L's': {
            const wchar_t *s = va_arg(ap, const wchar_t *);
            if (!s)
                s = L"(null)";
            size_t slen = wcslen(s);
            if (precision >= 0 && (size_t)precision < slen)
                slen = (size_t)precision;
            if (width > 0 && (int)slen < width) {
                for (int i = 0; i < width - (int)slen; i++)
                    wout_char(str, size, &pos, L' ');
            }
            wout_str(str, size, &pos, s, slen);
            break;
        }
        case L'd': {
            int v = va_arg(ap, int);
            sign = v < 0;
            len = wuint_to_base(sign ? (unsigned int)-v : (unsigned int)v,
                              10, 0, buf, sizeof(buf)/sizeof(wchar_t));
            break;
        }
        case L'u': {
            unsigned int v = va_arg(ap, unsigned int);
            len = wuint_to_base(v, 10, 0, buf, sizeof(buf)/sizeof(wchar_t));
            break;
        }
        case L'x':
        case L'X': {
            unsigned int v = va_arg(ap, unsigned int);
            len = wuint_to_base(v, 16, spec == L'X', buf, sizeof(buf)/sizeof(wchar_t));
            break;
        }
        case L'o': {
            unsigned int v = va_arg(ap, unsigned int);
            len = wuint_to_base(v, 8, 0, buf, sizeof(buf)/sizeof(wchar_t));
            break;
        }
        case L'p': {
            uintptr_t v = (uintptr_t)va_arg(ap, void *);
            prefix = L"0x";
            prefix_len = 2;
            len = wuint_to_base(v, 16, 0, buf, sizeof(buf)/sizeof(wchar_t));
            break;
        }
        case L'c': {
            buf[0] = (wchar_t)va_arg(ap, int);
            len = 1;
            break;
        }
        default:
            wout_char(str, size, &pos, L'%');
            if (spec)
                wout_char(str, size, &pos, spec);
            continue;
        }

        int num_len = len;
        if (precision > num_len)
            num_len = precision;
        int total = prefix_len + (sign ? 1 : 0) + num_len;
        if (width > total) {
            for (int i = 0; i < width - total; i++)
                wout_char(str, size, &pos, L' ');
        }
        if (sign)
            wout_char(str, size, &pos, L'-');
        wout_str(str, size, &pos, prefix, (size_t)prefix_len);
        for (int i = 0; i < num_len - len; i++)
            wout_char(str, size, &pos, L'0');
        wout_str(str, size, &pos, buf, (size_t)len);
    }

    if (size > 0) {
        if (pos >= size)
            str[size - 1] = L'\0';
        else
            str[pos] = L'\0';
    }
    return (int)pos;
}

int vswprintf(wchar_t *str, size_t size, const wchar_t *format, va_list ap)
{
    return vswprintf_impl(str, size, format, ap);
}

int swprintf(wchar_t *str, size_t size, const wchar_t *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int r = vswprintf(str, size, format, ap);
    va_end(ap);
    return r;
}

static int vfdwprintf(int fd, const wchar_t *format, va_list ap)
{
    wchar_t wbuf[1024];
    int len = vswprintf(wbuf, sizeof(wbuf)/sizeof(wchar_t), format, ap);
    if (len > 0 && fd >= 0) {
        char buf[4096];
        size_t out = wcstombs(buf, wbuf, sizeof(buf));
        if (out != (size_t)-1)
            write(fd, buf, out);
    }
    return len;
}

int vfwprintf(FILE *stream, const wchar_t *format, va_list ap)
{
    return vfdwprintf(stream ? stream->fd : -1, format, ap);
}

int vwprintf(const wchar_t *format, va_list ap)
{
    return vfdwprintf(1, format, ap);
}

int fwprintf(FILE *stream, const wchar_t *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int r = vfwprintf(stream, format, ap);
    va_end(ap);
    return r;
}

int wprintf(const wchar_t *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int r = vwprintf(format, ap);
    va_end(ap);
    return r;
}

