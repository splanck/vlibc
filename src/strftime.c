#include "time.h"
#include "string.h"

static int fmt_int(char *buf, size_t size, int val, int width)
{
    char tmp[16];
    int pos = 0;
    if (val < 0)
        val = -val;
    do {
        tmp[pos++] = '0' + (val % 10);
        val /= 10;
    } while (val && pos < (int)sizeof(tmp));
    while (pos < width && pos < (int)sizeof(tmp))
        tmp[pos++] = '0';
    if (pos > (int)size)
        pos = (int)size;
    for (int i = 0; i < pos; ++i)
        buf[i] = tmp[pos - i - 1];
    return pos;
}

size_t strftime(char *s, size_t max, const char *format, const struct tm *tm)
{
    if (!s || !format || !tm || max == 0)
        return 0;
    size_t pos = 0;
    for (const char *p = format; *p; ++p) {
        if (*p != '%') {
            if (pos + 1 >= max)
                return 0;
            s[pos++] = *p;
            continue;
        }
        ++p;
        if (!*p)
            break;
        char buf[16];
        int len = 0;
        switch (*p) {
        case '%':
            if (pos + 1 >= max)
                return 0;
            s[pos++] = '%';
            break;
        case 'Y':
            len = fmt_int(buf, sizeof(buf), tm->tm_year + 1900, 4);
            if (pos + (size_t)len >= max)
                return 0;
            memcpy(s + pos, buf, (size_t)len);
            pos += (size_t)len;
            break;
        case 'm':
            len = fmt_int(buf, sizeof(buf), tm->tm_mon + 1, 2);
            if (pos + (size_t)len >= max)
                return 0;
            memcpy(s + pos, buf, (size_t)len);
            pos += (size_t)len;
            break;
        case 'd':
            len = fmt_int(buf, sizeof(buf), tm->tm_mday, 2);
            if (pos + (size_t)len >= max)
                return 0;
            memcpy(s + pos, buf, (size_t)len);
            pos += (size_t)len;
            break;
        case 'H':
            len = fmt_int(buf, sizeof(buf), tm->tm_hour, 2);
            if (pos + (size_t)len >= max)
                return 0;
            memcpy(s + pos, buf, (size_t)len);
            pos += (size_t)len;
            break;
        case 'M':
            len = fmt_int(buf, sizeof(buf), tm->tm_min, 2);
            if (pos + (size_t)len >= max)
                return 0;
            memcpy(s + pos, buf, (size_t)len);
            pos += (size_t)len;
            break;
        case 'S':
            len = fmt_int(buf, sizeof(buf), tm->tm_sec, 2);
            if (pos + (size_t)len >= max)
                return 0;
            memcpy(s + pos, buf, (size_t)len);
            pos += (size_t)len;
            break;
        default:
            if (pos + 1 >= max)
                return 0;
            s[pos++] = '%';
            if (pos + 1 >= max)
                return 0;
            s[pos++] = *p;
            break;
        }
    }
    if (pos >= max)
        return 0;
    s[pos] = '\0';
    return pos;
}

