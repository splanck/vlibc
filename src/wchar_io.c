/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms, with or without modification, are permitted provided that the copyright notice and this permission notice appear in all copies. This software is provided "as is" without warranty.
 *
 * Purpose: Implements basic wide character I/O helpers for vlibc.
 */

#include "wchar.h"
#include "stdio.h"

wint_t fgetwc(FILE *stream)
{
    if (!stream)
        return -1;
    int c = fgetc(stream);
    if (c == -1)
        return -1;
    wchar_t wc = 0;
    if (mbrtowc(&wc, (char[]){(char)c, '\0'}, 1, NULL) == (size_t)-1)
        return -1;
    return (wint_t)wc;
}

wint_t fputwc(wchar_t wc, FILE *stream)
{
    if (!stream)
        return -1;
    char buf[8];
    size_t len = wcrtomb(buf, wc, NULL);
    if (len == (size_t)-1)
        return -1;
    if (fwrite(buf, 1, len, stream) != len)
        return -1;
    return (wint_t)wc;
}

