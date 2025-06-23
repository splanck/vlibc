#include "time.h"

static int parse_num(const char **sp, int width)
{
    const char *s = *sp;
    int val = 0;
    for (int i = 0; i < width; i++) {
        if (s[i] < '0' || s[i] > '9')
            return -1;
        val = val * 10 + (s[i] - '0');
    }
    *sp += width;
    return val;
}

char *strptime(const char *s, const char *format, struct tm *tm)
{
    if (!s || !format || !tm)
        return NULL;
    const char *p = s;
    for (; *format; ++format) {
        if (*format != '%') {
            if (*p != *format)
                return NULL;
            if (*p)
                p++;
            continue;
        }
        ++format;
        if (!*format)
            return NULL;
        int v;
        switch (*format) {
        case '%':
            if (*p != '%')
                return NULL;
            if (*p)
                p++;
            break;
        case 'Y':
            v = parse_num(&p, 4);
            if (v < 0)
                return NULL;
            tm->tm_year = v - 1900;
            break;
        case 'm':
            v = parse_num(&p, 2);
            if (v < 1 || v > 12)
                return NULL;
            tm->tm_mon = v - 1;
            break;
        case 'd':
            v = parse_num(&p, 2);
            if (v < 1 || v > 31)
                return NULL;
            tm->tm_mday = v;
            break;
        case 'H':
            v = parse_num(&p, 2);
            if (v < 0 || v > 23)
                return NULL;
            tm->tm_hour = v;
            break;
        case 'M':
            v = parse_num(&p, 2);
            if (v < 0 || v > 59)
                return NULL;
            tm->tm_min = v;
            break;
        case 'S':
            v = parse_num(&p, 2);
            if (v < 0 || v > 60)
                return NULL;
            tm->tm_sec = v;
            break;
        default:
            return NULL;
        }
    }
    return (char *)p;
}

