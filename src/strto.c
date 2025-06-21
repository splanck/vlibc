#include "stdlib.h"
#include "string.h"

static int digit_val(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'z')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'Z')
        return c - 'A' + 10;
    return -1;
}

long strtol(const char *nptr, char **endptr, int base)
{
    const char *s = nptr;
    while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r' || *s == '\f' || *s == '\v')
        s++;

    int neg = 0;
    if (*s == '+' || *s == '-') {
        if (*s == '-')
            neg = 1;
        s++;
    }

    if (base == 0) {
        if (*s == '0') {
            if (s[1] == 'x' || s[1] == 'X') {
                base = 16;
                s += 2;
            } else {
                base = 8;
                s += 1;
            }
        } else {
            base = 10;
        }
    } else if (base == 16) {
        if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
            s += 2;
    }

    unsigned long val = 0;
    const char *start = s;
    int d;
    while ((d = digit_val(*s)) >= 0 && d < base) {
        val = val * (unsigned long)base + (unsigned long)d;
        s++;
    }

    if (endptr)
        *endptr = (char *)(s != start ? s : nptr);

    if (s == start)
        return 0;

    long result = (long)val;
    if (neg)
        result = -result;
    return result;
}

int atoi(const char *nptr)
{
    return (int)strtol(nptr, NULL, 10);
}

