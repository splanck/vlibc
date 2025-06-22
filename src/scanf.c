#include "stdio.h"
#include "ctype.h"
#include "string.h"
#include "stdlib.h"

static const char *skip_ws(const char *s)
{
    while (*s && isspace((unsigned char)*s))
        s++;
    return s;
}

static int vsscanf_impl(const char *str, const char *fmt, va_list ap)
{
    const char *s = str;
    int count = 0;
    for (; *fmt; ++fmt) {
        if (isspace((unsigned char)*fmt)) {
            while (isspace((unsigned char)*fmt))
                fmt++;
            fmt--;
            s = skip_ws(s);
            continue;
        }
        if (*fmt != '%') {
            if (*s != *fmt)
                return count;
            if (*s)
                s++;
            continue;
        }
        fmt++;
        if (*fmt == 'd') {
            s = skip_ws(s);
            char *end;
            long v = strtol(s, &end, 10);
            if (end == s)
                return count;
            *va_arg(ap, int *) = (int)v;
            s = end;
            count++;
        } else if (*fmt == 'u') {
            s = skip_ws(s);
            char *end;
            long v = strtol(s, &end, 10);
            if (end == s || v < 0)
                return count;
            *va_arg(ap, unsigned *) = (unsigned)v;
            s = end;
            count++;
        } else if (*fmt == 's') {
            s = skip_ws(s);
            char *out = va_arg(ap, char *);
            if (!*s)
                return count;
            while (*s && !isspace((unsigned char)*s))
                *out++ = *s++;
            *out = '\0';
            count++;
        } else if (*fmt == '%') {
            if (*s != '%')
                return count;
            if (*s)
                s++;
        } else {
            return count;
        }
    }
    return count;
}

int vsscanf(const char *str, const char *format, va_list ap)
{
    return vsscanf_impl(str, format, ap);
}

int sscanf(const char *str, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int r = vsscanf_impl(str, format, ap);
    va_end(ap);
    return r;
}

static int vfscanf_impl(FILE *stream, const char *format, va_list ap)
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
    return vsscanf_impl(buf, format, ap);
}

int vfscanf(FILE *stream, const char *format, va_list ap)
{
    return vfscanf_impl(stream, format, ap);
}

int fscanf(FILE *stream, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int r = vfscanf_impl(stream, format, ap);
    va_end(ap);
    return r;
}

int scanf(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int r = vfscanf_impl(stdin, format, ap);
    va_end(ap);
    return r;
}

int vscanf(const char *format, va_list ap)
{
    return vfscanf_impl(stdin, format, ap);
}
