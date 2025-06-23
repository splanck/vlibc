#include "stdio.h"
#include "wchar.h"
#include "wctype.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

static const wchar_t *skip_ws_w(const wchar_t *s)
{
    while (*s && iswspace(*s))
        s++;
    return s;
}

static long wstrtol_wrap(const wchar_t *s, const wchar_t **end, int base)
{
    char buf[128];
    size_t len = wcstombs(buf, s, sizeof(buf) - 1);
    if (len == (size_t)-1)
        len = 0;
    buf[len] = '\0';
    char *endp;
    long v = strtol(buf, &endp, base);
    if (end)
        *end = s + (endp - buf);
    return v;
}

static unsigned long wstrtoul_wrap(const wchar_t *s, const wchar_t **end, int base)
{
    char buf[128];
    size_t len = wcstombs(buf, s, sizeof(buf) - 1);
    if (len == (size_t)-1)
        len = 0;
    buf[len] = '\0';
    char *endp;
    unsigned long v = strtoul(buf, &endp, base);
    if (end)
        *end = s + (endp - buf);
    return v;
}

static double wstrtod_wrap(const wchar_t *s, const wchar_t **end)
{
    char buf[128];
    size_t len = wcstombs(buf, s, sizeof(buf) - 1);
    if (len == (size_t)-1)
        len = 0;
    buf[len] = '\0';
    char *endp;
    double v = strtod(buf, &endp);
    if (end)
        *end = s + (endp - buf);
    return v;
}

static int vswscanf_impl(const wchar_t *str, const wchar_t *fmt, va_list ap)
{
    const wchar_t *s = str;
    int count = 0;
    for (; *fmt; ++fmt) {
        if (iswspace(*fmt)) {
            while (iswspace(*fmt))
                fmt++;
            fmt--;
            s = skip_ws_w(s);
            continue;
        }
        if (*fmt != L'%') {
            if (*s != *fmt)
                return count;
            if (*s)
                s++;
            continue;
        }
        fmt++;
        int long_mod = 0;
        if (*fmt == L'l') {
            long_mod = 1;
            fmt++;
        }
        if (*fmt == L'd') {
            s = skip_ws_w(s);
            const wchar_t *end;
            long v = wstrtol_wrap(s, &end, 10);
            if (end == s)
                return count;
            *va_arg(ap, int *) = (int)v;
            s = end;
            count++;
        } else if (*fmt == L'u') {
            s = skip_ws_w(s);
            const wchar_t *end;
            long v = wstrtol_wrap(s, &end, 10);
            if (end == s || v < 0)
                return count;
            *va_arg(ap, unsigned *) = (unsigned)v;
            s = end;
            count++;
        } else if (*fmt == L'x' || *fmt == L'X') {
            s = skip_ws_w(s);
            const wchar_t *end;
            unsigned long v = wstrtoul_wrap(s, &end, 16);
            if (end == s)
                return count;
            *va_arg(ap, unsigned *) = (unsigned)v;
            s = end;
            count++;
        } else if (*fmt == L'o') {
            s = skip_ws_w(s);
            const wchar_t *end;
            unsigned long v = wstrtoul_wrap(s, &end, 8);
            if (end == s)
                return count;
            *va_arg(ap, unsigned *) = (unsigned)v;
            s = end;
            count++;
        } else if (*fmt == L'f' || *fmt == L'F' ||
                   *fmt == L'e' || *fmt == L'E' ||
                   *fmt == L'g' || *fmt == L'G') {
            s = skip_ws_w(s);
            const wchar_t *end;
            double v = wstrtod_wrap(s, &end);
            if (end == s)
                return count;
            if (long_mod)
                *va_arg(ap, double *) = v;
            else
                *va_arg(ap, float *) = (float)v;
            s = end;
            count++;
        } else if (*fmt == L's') {
            s = skip_ws_w(s);
            wchar_t *out = va_arg(ap, wchar_t *);
            if (!*s)
                return count;
            while (*s && !iswspace(*s))
                *out++ = *s++;
            *out = L'\0';
            count++;
        } else if (*fmt == L'%') {
            if (*s != L'%')
                return count;
            if (*s)
                s++;
        } else {
            return count;
        }
    }
    return count;
}

int vswscanf(const wchar_t *str, const wchar_t *format, va_list ap)
{
    return vswscanf_impl(str, format, ap);
}

int swscanf(const wchar_t *str, const wchar_t *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int r = vswscanf_impl(str, format, ap);
    va_end(ap);
    return r;
}

static int vfwscanf_impl(FILE *stream, const wchar_t *format, va_list ap)
{
    char buf[256];
    size_t pos = 0;
    int c;
    while (pos + 1 < sizeof(buf) && (c = fgetc(stream)) != -1) {
        buf[pos++] = (char)c;
        if (c == '\n')
            break;
    }
    buf[pos] = '\0';
    wchar_t wbuf[256];
    mbstowcs(wbuf, buf, sizeof(wbuf)/sizeof(wchar_t));
    return vswscanf_impl(wbuf, format, ap);
}

int vfwscanf(FILE *stream, const wchar_t *format, va_list ap)
{
    return vfwscanf_impl(stream, format, ap);
}

int vwscanf(const wchar_t *format, va_list ap)
{
    return vfwscanf_impl(stdin, format, ap);
}

int fwscanf(FILE *stream, const wchar_t *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int r = vfwscanf(stream, format, ap);
    va_end(ap);
    return r;
}

int wscanf(const wchar_t *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int r = vfwscanf_impl(stdin, format, ap);
    va_end(ap);
    return r;
}


