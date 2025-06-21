#include "stdio.h"
#include "io.h"
#include <stdarg.h>
#include <string.h>

static int uint_to_str(unsigned int value, char *buf, size_t size)
{
    char tmp[20];
    size_t i = 0;
    do {
        tmp[i++] = '0' + (value % 10);
        value /= 10;
    } while (value && i < sizeof(tmp));
    if (i > size)
        i = size;
    for (size_t j = 0; j < i; ++j)
        buf[j] = tmp[i - j - 1];
    return (int)i;
}

static int int_to_str(int value, char *buf, size_t size)
{
    unsigned int v = (value < 0) ? -value : value;
    size_t pos = 0;
    if (value < 0) {
        if (pos < size)
            buf[pos] = '-';
        pos++;
    }
    pos += uint_to_str(v, buf + pos, (pos < size) ? size - pos : 0);
    return (int)pos;
}

static int vsnprintf_impl(char *str, size_t size, const char *fmt, va_list ap)
{
    size_t pos = 0;
    for (const char *p = fmt; *p; ++p) {
        if (*p != '%') {
            if (pos + 1 < size)
                str[pos] = *p;
            pos++;
            continue;
        }
        ++p;
        if (*p == '%') {
            if (pos + 1 < size)
                str[pos] = '%';
            pos++;
        } else if (*p == 's') {
            const char *s = va_arg(ap, const char *);
            if (!s)
                s = "(null)";
            while (*s) {
                if (pos + 1 < size)
                    str[pos] = *s;
                pos++; s++; }
        } else if (*p == 'd') {
            char num[32];
            int n = int_to_str(va_arg(ap, int), num, sizeof(num));
            for (int i = 0; i < n; ++i) {
                if (pos + 1 < size)
                    str[pos] = num[i];
                pos++; }
        } else if (*p == 'u') {
            char num[32];
            int n = uint_to_str(va_arg(ap, unsigned int), num, sizeof(num));
            for (int i = 0; i < n; ++i) {
                if (pos + 1 < size)
                    str[pos] = num[i];
                pos++; }
        } else {
            /* unsupported specifier, output as-is */
            if (pos + 1 < size)
                str[pos] = '%';
            pos++;
            if (*p) {
                if (pos + 1 < size)
                    str[pos] = *p;
                pos++; }
        }
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

static int vfdprintf(int fd, const char *format, va_list ap)
{
    char buf[1024];
    int len = vsnprintf(buf, sizeof(buf), format, ap);
    if (len > 0 && fd >= 0)
        write(fd, buf, (size_t)len);
    return len;
}

int vfprintf(FILE *stream, const char *format, va_list ap)
{
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

int printf(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int r = vprintf(format, ap);
    va_end(ap);
    return r;
}

