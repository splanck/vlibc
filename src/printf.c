/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms, with or without modification, are permitted provided that the copyright notice and this permission notice appear in all copies. This software is provided "as is" without warranty.
 *
 * Purpose: Implements the printf functions for vlibc. Provides wrappers and helpers used by the standard library.
 *
 * Copyright (c) 2025
 */

#include "stdio.h"
#include "io.h"
#include "memory.h"
#include "errno.h"
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>

static int uint_to_base(unsigned long value, unsigned base, int upper,
                        char *buf, size_t size)
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
        buf[j] = tmp[i - j - 1];
    return (int)i;
}

static int ull_to_base(unsigned long long value, unsigned base, int upper,
                       char *buf, size_t size)
{
    const char *digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    char tmp[64];
    size_t i = 0;
    do {
        tmp[i++] = digits[value % base];
        value /= base;
    } while (value && i < sizeof(tmp));
    if (i > size)
        i = size;
    for (size_t j = 0; j < i; ++j)
        buf[j] = tmp[i - j - 1];
    return (int)i;
}


static void out_char(char *dst, size_t size, size_t *pos, char c)
{
    if (*pos + 1 < size)
        dst[*pos] = c;
    (*pos)++;
}

static void out_str(char *dst, size_t size, size_t *pos,
                    const char *s, size_t len)
{
    for (size_t i = 0; i < len; i++)
        out_char(dst, size, pos, s[i]);
}

/*
 * vsnprintf_impl handles the core of formatted output. It parses the format
 * string and builds a temporary result in the caller provided buffer. Only a
 * small subset of printf formatting is implemented.
 */
static int vsnprintf_impl(char *str, size_t size, const char *fmt, va_list ap)
{
    size_t pos = 0;
    for (const char *p = fmt; *p; ++p) {
        if (*p != '%') {
            out_char(str, size, &pos, *p);
            continue;
        }
        ++p;
        if (*p == '%') {
            out_char(str, size, &pos, '%');
            continue;
        }

        int left = 0, plus = 0, spacef = 0, zero = 0, alt = 0;
        int width = 0;
        int precision = -1;
        for (;; ++p) {
            if (*p == '-') { left = 1; continue; }
            if (*p == '+') { plus = 1; continue; }
            if (*p == ' ') { spacef = 1; continue; }
            if (*p == '0') { zero = 1; continue; }
            if (*p == '#') { alt = 1; continue; }
            break;
        }
        while (*p >= '0' && *p <= '9') {
            width = width * 10 + (*p - '0');
            ++p;
        }
        if (*p == '.') {
            ++p;
            precision = 0;
            while (*p >= '0' && *p <= '9') {
                precision = precision * 10 + (*p - '0');
                ++p;
            }
        }

        enum { LEN_NONE, LEN_HH, LEN_H, LEN_L, LEN_LL, LEN_J, LEN_Z,
               LEN_T, LEN_LD } length = LEN_NONE;
        if (*p == 'h') {
            length = LEN_H;
            ++p;
            if (*p == 'h') {
                length = LEN_HH;
                ++p;
            }
        } else if (*p == 'l') {
            length = LEN_L;
            ++p;
            if (*p == 'l') {
                length = LEN_LL;
                ++p;
            }
        } else if (*p == 'j') {
            length = LEN_J;
            ++p;
        } else if (*p == 'z') {
            length = LEN_Z;
            ++p;
        } else if (*p == 't') {
            length = LEN_T;
            ++p;
        } else if (*p == 'L') {
            length = LEN_LD;
            ++p;
        }

        char spec = *p;
        char buf[64];
        int len = 0;
        const char *prefix = "";
        int prefix_len = 0;
        int sign = 0;
        char sign_char = 0;

        switch (spec) {
        case 's': {
            const char *s = va_arg(ap, const char *);
            if (!s)
                s = "(null)";
            size_t slen = strlen(s);
            if (precision >= 0 && (size_t)precision < slen)
                slen = (size_t)precision;
            if (width > (int)slen) {
                if (left) {
                    out_str(str, size, &pos, s, slen);
                    for (int i = 0; i < width - (int)slen; i++)
                        out_char(str, size, &pos, ' ');
                } else {
                    char pad = zero ? '0' : ' ';
                    for (int i = 0; i < width - (int)slen; i++)
                        out_char(str, size, &pos, pad);
                    out_str(str, size, &pos, s, slen);
                }
            } else {
                out_str(str, size, &pos, s, slen);
            }
            break;
        }
        case 'd': {
            long long sv = 0;
            switch (length) {
            case LEN_HH: sv = (signed char)va_arg(ap, int); break;
            case LEN_H:  sv = (short)va_arg(ap, int); break;
            case LEN_L:  sv = va_arg(ap, long); break;
            case LEN_LL: sv = va_arg(ap, long long); break;
            case LEN_J:  sv = va_arg(ap, intmax_t); break;
            case LEN_Z:  sv = (ssize_t)va_arg(ap, ssize_t); break;
            case LEN_T:  sv = va_arg(ap, ptrdiff_t); break;
            default:     sv = va_arg(ap, int); break;
            }
            unsigned long long uv = (unsigned long long)sv;
            sign = sv < 0;
            if (sign)
                uv = (unsigned long long)(-sv);
            if (sign)
                sign_char = '-';
            else if (plus)
                sign_char = '+';
            else if (spacef)
                sign_char = ' ';
            len = ull_to_base(uv, 10, 0, buf, sizeof(buf));
            break;
        }
        case 'u': {
            unsigned long long uv = 0;
            switch (length) {
            case LEN_HH: uv = (unsigned char)va_arg(ap, int); break;
            case LEN_H:  uv = (unsigned short)va_arg(ap, int); break;
            case LEN_L:  uv = va_arg(ap, unsigned long); break;
            case LEN_LL: uv = va_arg(ap, unsigned long long); break;
            case LEN_J:  uv = (unsigned long long)va_arg(ap, uintmax_t); break;
            case LEN_Z:  uv = va_arg(ap, size_t); break;
            case LEN_T:  uv = (unsigned long long)va_arg(ap, ptrdiff_t); break;
            default:     uv = va_arg(ap, unsigned int); break;
            }
            len = ull_to_base(uv, 10, 0, buf, sizeof(buf));
            break;
        }
        case 'x':
        case 'X': {
            unsigned long long uv = 0;
            switch (length) {
            case LEN_HH: uv = (unsigned char)va_arg(ap, int); break;
            case LEN_H:  uv = (unsigned short)va_arg(ap, int); break;
            case LEN_L:  uv = va_arg(ap, unsigned long); break;
            case LEN_LL: uv = va_arg(ap, unsigned long long); break;
            case LEN_J:  uv = (unsigned long long)va_arg(ap, uintmax_t); break;
            case LEN_Z:  uv = va_arg(ap, size_t); break;
            case LEN_T:  uv = (unsigned long long)va_arg(ap, ptrdiff_t); break;
            default:     uv = va_arg(ap, unsigned int); break;
            }
            if (alt && uv != 0) {
                prefix = (spec == 'X') ? "0X" : "0x";
                prefix_len = 2;
            }
            len = ull_to_base(uv, 16, spec == 'X', buf, sizeof(buf));
            break;
        }
        case 'o': {
            unsigned long long uv = 0;
            switch (length) {
            case LEN_HH: uv = (unsigned char)va_arg(ap, int); break;
            case LEN_H:  uv = (unsigned short)va_arg(ap, int); break;
            case LEN_L:  uv = va_arg(ap, unsigned long); break;
            case LEN_LL: uv = va_arg(ap, unsigned long long); break;
            case LEN_J:  uv = (unsigned long long)va_arg(ap, uintmax_t); break;
            case LEN_Z:  uv = va_arg(ap, size_t); break;
            case LEN_T:  uv = (unsigned long long)va_arg(ap, ptrdiff_t); break;
            default:     uv = va_arg(ap, unsigned int); break;
            }
            len = ull_to_base(uv, 8, 0, buf, sizeof(buf));
            if (alt && (uv != 0 || precision == 0)) {
                prefix = "0";
                prefix_len = 1;
            }
            break;
        }
        case 'p': {
            uintptr_t v = (uintptr_t)va_arg(ap, void *);
            prefix = "0x";
            prefix_len = 2;
            len = uint_to_base(v, 16, 0, buf, sizeof(buf));
            break;
        }
        case 'c': {
            buf[0] = (char)va_arg(ap, int);
            len = 1;
            break;
        }
        default:
            out_char(str, size, &pos, '%');
            if (spec)
                out_char(str, size, &pos, spec);
            continue;
        }

        int num_len = len;
        if (precision > num_len)
            num_len = precision;
        int prefix_total = prefix_len + (sign_char ? 1 : 0);
        int zero_pad = 0;
        if (zero && precision < 0 && !left && width > prefix_total + num_len)
            zero_pad = width - (prefix_total + num_len);
        int total = prefix_total + zero_pad + num_len;
        int spaces_pre = (!left && width > total) ? width - total : 0;
        int spaces_post = (left && width > total) ? width - total : 0;
        for (int i = 0; i < spaces_pre; i++)
            out_char(str, size, &pos, ' ');
        if (sign_char)
            out_char(str, size, &pos, sign_char);
        out_str(str, size, &pos, prefix, (size_t)prefix_len);
        for (int i = 0; i < zero_pad; i++)
            out_char(str, size, &pos, '0');
        for (int i = 0; i < num_len - len; i++)
            out_char(str, size, &pos, '0');
        out_str(str, size, &pos, buf, (size_t)len);
        for (int i = 0; i < spaces_post; i++)
            out_char(str, size, &pos, ' ');
    }

    if (size > 0) {
        if (pos >= size)
            str[size - 1] = '\0';
        else
            str[pos] = '\0';
    }
    return (int)pos;
}

int vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
    return vsnprintf_impl(str, size, format, ap);
}

int snprintf(char *str, size_t size, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int r = vsnprintf(str, size, format, ap);
    va_end(ap);
    return r;
}

int vsprintf(char *str, const char *format, va_list ap)
{
    return vsnprintf(str, (size_t)-1, format, ap);
}

int sprintf(char *str, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int r = vsprintf(str, format, ap);
    va_end(ap);
    return r;
}

/*
 * vfdprintf formats to a fixed-size buffer and writes the result to a
 * file descriptor. It is the low level helper used by printf and friends.
 */
static int vfdprintf(int fd, const char *format, va_list ap)
{
    char buf[1024];
    int len = vsnprintf(buf, sizeof(buf), format, ap);
    if (len > 0 && fd >= 0) {
        size_t off = 0;
        while (off < (size_t)len) {
            ssize_t w = write(fd, buf + off, (size_t)len - off);
            if (w < 0) {
                if (errno == EINTR || errno == EAGAIN)
                    continue;
                return -1;
            }
            off += (size_t)w;
        }
    }
    return len;
}

int vdprintf(int fd, const char *format, va_list ap)
{
    return vfdprintf(fd, format, ap);
}

int dprintf(int fd, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int r = vdprintf(fd, format, ap);
    va_end(ap);
    return r;
}

/*
 * vfprintf sends formatted output to a FILE stream. Memory-backed streams
 * are handled specially by formatting into a temporary buffer and then
 * writing with fwrite(). For regular files, vfdprintf performs the work.
 */
int vfprintf(FILE *stream, const char *format, va_list ap)
{
    if (stream && stream->is_mem) {
        va_list copy;
        va_copy(copy, ap);
        int len = vsnprintf(NULL, 0, format, copy);
        va_end(copy);
        if (len < 0)
            return -1;
        char *buf = malloc((size_t)len + 1);
        if (!buf) {
            errno = ENOMEM;
            return -1;
        }
        vsnprintf(buf, (size_t)len + 1, format, ap);
        size_t w = fwrite(buf, 1, (size_t)len, stream);
        free(buf);
        return (int)w;
    }
    return vfdprintf(stream ? stream->fd : -1, format, ap);
}

int vprintf(const char *format, va_list ap)
{
    return vfdprintf(1, format, ap);
}

int fprintf(FILE *stream, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int r = vfprintf(stream, format, ap);
    va_end(ap);
    return r;
}

/*
 * printf formats the arguments according to the format string and writes the
 * result to standard output. It delegates most of the work to vprintf.
 */
int printf(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int r = vprintf(format, ap);
    va_end(ap);
    return r;
}

int vasprintf(char **strp, const char *format, va_list ap)
{
    va_list copy;
    va_copy(copy, ap);
    int len = vsnprintf(NULL, 0, format, copy);
    va_end(copy);
    if (len < 0) {
        if (strp)
            *strp = NULL;
        return -1;
    }
    char *buf = malloc((size_t)len + 1);
    if (!buf) {
        if (strp)
            *strp = NULL;
        errno = ENOMEM;
        return -1;
    }
    int r = vsnprintf(buf, (size_t)len + 1, format, ap);
    if (r < 0) {
        free(buf);
        if (strp)
            *strp = NULL;
        return -1;
    }
    if (strp)
        *strp = buf;
    else
        free(buf);
    return r;
}

int asprintf(char **strp, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int r = vasprintf(strp, format, ap);
    va_end(ap);
    return r;
}

