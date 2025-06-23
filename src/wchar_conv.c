#include "wchar.h"
#include_next <wchar.h>

extern size_t host_mbrtowc(wchar_t *, const char *, size_t, mbstate_t *) __asm__("mbrtowc");
extern size_t host_wcrtomb(char *, wchar_t, mbstate_t *) __asm__("wcrtomb");
extern size_t host_mbstowcs(wchar_t *, const char *, size_t) __asm__("mbstowcs");
extern size_t host_wcstombs(char *, const wchar_t *, size_t) __asm__("wcstombs");

int mbsinit(const mbstate_t *ps)
{
    (void)ps;
    return 1;
}

size_t mbrlen(const char *s, size_t n, mbstate_t *ps)
{
    return mbrtowc(NULL, s, n, ps);
}

size_t mbrtowc(wchar_t *pwc, const char *s, size_t n, mbstate_t *ps)
{
    (void)ps;
    if (!s)
        return 0;
    if (n == 0)
        return (size_t)-2;
    unsigned char ch = (unsigned char)*s;
    if (ch < 0x80) {
        if (pwc)
            *pwc = (wchar_t)ch;
        return ch ? 1 : 0;
    }
    return host_mbrtowc(pwc, s, n, ps);
}

size_t wcrtomb(char *s, wchar_t wc, mbstate_t *ps)
{
    (void)ps;
    if (!s)
        return 1;
    if ((unsigned)wc < 0x80) {
        *s = (char)wc;
        return 1;
    }
    return host_wcrtomb(s, wc, ps);
}

static int has_non_ascii_mb(const char *s)
{
    while (*s) {
        if (((unsigned char)*s) >= 0x80)
            return 1;
        s++;
    }
    return 0;
}

static int has_non_ascii_wc(const wchar_t *s)
{
    while (*s) {
        if ((unsigned)*s >= 0x80)
            return 1;
        s++;
    }
    return 0;
}

size_t mbstowcs(wchar_t *dst, const char *src, size_t n)
{
    if (has_non_ascii_mb(src))
        return host_mbstowcs(dst, src, n);

    size_t i = 0;
    if (dst) {
        for (; i < n && src[i]; i++)
            dst[i] = (unsigned char)src[i];
        if (i < n)
            dst[i] = 0;
    } else {
        while (src[i])
            i++;
    }
    return i;
}

size_t wcstombs(char *dst, const wchar_t *src, size_t n)
{
    if (has_non_ascii_wc(src))
        return host_wcstombs(dst, src, n);

    size_t i = 0;
    if (dst) {
        for (; i < n && src[i]; i++)
            dst[i] = (char)src[i];
        if (i < n)
            dst[i] = 0;
    } else {
        while (src[i])
            i++;
    }
    return i;
}
