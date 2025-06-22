#include "stdio.h"
#include "io.h"
#include <stdarg.h>
#include <string.h>
#include <stdint.h>

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

static int uint_to_hex(unsigned long value, char *buf, size_t size, int upper)
{
    char tmp[32];
    size_t i = 0;
    do {
        unsigned d = (unsigned)(value & 0xF);
        if (d < 10)
            tmp[i++] = '0' + d;
        else
            tmp[i++] = (upper ? 'A' : 'a') + (d - 10);
        value >>= 4;
    } while (value && i < sizeof(tmp));
    if (i > size)
        i = size;
    for (size_t j = 0; j < i; ++j)
        buf[j] = tmp[i - j - 1];
    return (int)i;
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
        int width = 0;
        int precision = -1;
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
        if (*p == '%') {
            if (pos + 1 < size)
                str[pos] = '%';
            pos++;
        } else if (*p == 's') {
            const char *s = va_arg(ap, const char *);
            if (!s)
                s = "(null)";
            size_t len = strlen(s);
            if (precision >= 0 && (size_t)precision < len)
                len = (size_t)precision;
            int pad = width > (int)len ? width - (int)len : 0;
            for (int i = 0; i < pad; ++i) {
                if (pos + 1 < size)
                    str[pos] = ' ';
                pos++;
            }
            for (size_t i = 0; i < len; ++i) {
                if (pos + 1 < size)
                    str[pos] = s[i];
                pos++;
            }
        } else if (*p == 'd') {
            char num[32];
            int value = va_arg(ap, int);
            int n = int_to_str(value, num, sizeof(num));
            int sign = (value < 0) ? 1 : 0;
            int digits = n - sign;
            int zeroes = 0;
            if (precision > digits)
                zeroes = precision - digits;
            int total = digits + zeroes + sign;
            if (width > total)
                for (int i = 0; i < width - total; ++i) {
                    if (pos + 1 < size)
                        str[pos] = ' ';
                    pos++;
                }
            int idx = sign;
            if (sign) {
                if (pos + 1 < size)
                    str[pos] = '-';
                pos++;
            }
            for (int i = 0; i < zeroes; ++i) {
                if (pos + 1 < size)
                    str[pos] = '0';
                pos++;
            }
            for (; idx < n; ++idx) {
                if (pos + 1 < size)
                    str[pos] = num[idx];
                pos++;
            }
        } else if (*p == 'u') {
            char num[32];
            unsigned int v = va_arg(ap, unsigned int);
            int n = uint_to_str(v, num, sizeof(num));
            int zeroes = 0;
            if (precision > n)
                zeroes = precision - n;
            int total = n + zeroes;
            if (width > total)
                for (int i = 0; i < width - total; ++i) {
                    if (pos + 1 < size)
                        str[pos] = ' ';
                    pos++;
                }
            for (int i = 0; i < zeroes; ++i) {
                if (pos + 1 < size)
                    str[pos] = '0';
                pos++;
            }
            for (int i = 0; i < n; ++i) {
                if (pos + 1 < size)
                    str[pos] = num[i];
                pos++;
            }
        } else if (*p == 'x' || *p == 'X') {
            char num[32];
            unsigned int v = va_arg(ap, unsigned int);
            int n = uint_to_hex(v, num, sizeof(num), *p == 'X');
            int zeroes = 0;
            if (precision > n)
                zeroes = precision - n;
            int total = n + zeroes;
            if (width > total)
                for (int i = 0; i < width - total; ++i) {
                    if (pos + 1 < size)
                        str[pos] = ' ';
                    pos++;
                }
            for (int i = 0; i < zeroes; ++i) {
                if (pos + 1 < size)
                    str[pos] = '0';
                pos++;
            }
            for (int i = 0; i < n; ++i) {
                if (pos + 1 < size)
                    str[pos] = num[i];
                pos++;
            }
        } else if (*p == 'c') {
            char ch = (char)va_arg(ap, int);
            int pad = width > 1 ? width - 1 : 0;
            for (int i = 0; i < pad; ++i) {
                if (pos + 1 < size)
                    str[pos] = ' ';
                pos++;
            }
            if (pos + 1 < size)
                str[pos] = ch;
            pos++;
        } else if (*p == 'p') {
            char num[32];
            uintptr_t v = (uintptr_t)va_arg(ap, void *);
            int n = uint_to_hex(v, num, sizeof(num), 0);
            int total = n + 2; /* 0x prefix */
            int pad = width > total ? width - total : 0;
            for (int i = 0; i < pad; ++i) {
                if (pos + 1 < size)
                    str[pos] = ' ';
                pos++;
            }
            if (pos + 1 < size) str[pos] = '0';
            pos++;
            if (pos + 1 < size) str[pos] = 'x';
            pos++;
            for (int i = 0; i < n; ++i) {
                if (pos + 1 < size)
                    str[pos] = num[i];
                pos++;
            }
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

