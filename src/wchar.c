/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms, with or without modification, are permitted provided that the copyright notice and this permission notice appear in all copies. This software is provided "as is" without warranty.
 *
 * Purpose: Implements the wchar functions for vlibc. Provides wrappers and helpers used by the standard library.
 *
 * Copyright (c) 2025
 */

#include "wchar.h"
#include "memory.h"

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

wchar_t *wcscpy(wchar_t *dest, const wchar_t *src)
{
    wchar_t *d = dest;
    while ((*d++ = *src++) != 0)
        ;
    return dest;
}

wchar_t *wcsncpy(wchar_t *dest, const wchar_t *src, size_t n)
{
    wchar_t *d = dest;
    while (n && *src) {
        *d++ = *src++;
        --n;
    }
    while (n--) {
        *d++ = 0;
    }
    return dest;
}

int wcscmp(const wchar_t *s1, const wchar_t *s2)
{
    while (*s1 && *s1 == *s2) {
        s1++;
        s2++;
    }
    return (int)(*s1 - *s2);
}

int wcsncmp(const wchar_t *s1, const wchar_t *s2, size_t n)
{
    while (n--) {
        wchar_t c1 = *s1++;
        wchar_t c2 = *s2++;
        if (c1 != c2)
            return (int)(c1 - c2);
        if (c1 == 0)
            break;
    }
    return 0;
}

wchar_t *wcsdup(const wchar_t *s)
{
    size_t len = wcslen(s);
    wchar_t *dup = malloc((len + 1) * sizeof(wchar_t));
    if (!dup)
        return NULL;
    wcscpy(dup, s);
    return dup;
}
