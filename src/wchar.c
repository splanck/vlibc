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

int wcwidth(wchar_t wc)
{
    if (wc == 0)
        return 0;
    if (wc >= 0x20 && wc < 0x7f)
        return 1;
    if (wc < 0x20 || wc == 0x7f)
        return -1;
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    extern int host_wcwidth(wchar_t) __asm("wcwidth");
    return host_wcwidth(wc);
#else
    (void)wc;
    return -1;
#endif
}

int wcswidth(const wchar_t *s, size_t n)
{
    int width = 0;
    for (size_t i = 0; i < n && s[i]; i++) {
        int w = wcwidth(s[i]);
        if (w < 0)
            return -1;
        width += w;
    }
    return width;
}
