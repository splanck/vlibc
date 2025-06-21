#include "wchar.h"

int mbtowc(wchar_t *pwc, const char *s, size_t n)
{
    if (!s)
        return 0;
    if (n == 0)
        return -1;
    if (pwc)
        *pwc = (unsigned char)*s;
    return *s ? 1 : 0;
}

int wctomb(char *s, wchar_t wc)
{
    if (!s)
        return 0;
    if (wc < 0 || wc > 0xFF)
        return -1;
    *s = (char)wc;
    return 1;
}

size_t wcslen(const wchar_t *s)
{
    const wchar_t *p = s;
    while (*p)
        p++;
    return (size_t)(p - s);
}
