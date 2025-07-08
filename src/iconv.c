/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms, with or without modification, are permitted provided that the copyright notice and this permission notice appear in all copies. This software is provided "as is" without warranty.
 *
 * Purpose: Implements the iconv functions for vlibc. Provides wrappers and helpers used by the standard library.
 *
 * Copyright (c) 2025
 */

#include "iconv.h"
#include "string.h"
#include "stdlib.h"
#include "errno.h"

#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
#define iconv_open host_iconv_open
#define iconv host_iconv_func
#define iconv_close host_iconv_close
#include "host/iconv.h"
#undef iconv_open
#undef iconv
#undef iconv_close
extern iconv_t host_iconv_open(const char *, const char *);
extern size_t host_iconv(iconv_t, char **, size_t *, char **, size_t *);
extern int host_iconv_close(iconv_t);
#endif

enum {
    CD_ASCII_ASCII,
    CD_ASCII_UTF8,
    CD_UTF8_ASCII,
    CD_UTF8_UTF8,
    CD_ASCII_8859_1,
    CD_8859_1_ASCII,
    CD_8859_1_8859_1,
    CD_8859_1_UTF8,
    CD_UTF8_8859_1,
    CD_ASCII_UTF16,
    CD_UTF16_ASCII,
    CD_UTF8_UTF16,
    CD_UTF16_UTF8,
    CD_8859_1_UTF16,
    CD_UTF16_8859_1,
    CD_UTF16_UTF16,
    CD_HOST
};

typedef struct iconv_cd {
    int type;
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    iconv_t host;
#endif
} iconv_cd;

/* Determine whether a character set name refers to ASCII. */
static int is_ascii_cs(const char *name)
{
    return name && (!strcasecmp(name, "ASCII") || !strcasecmp(name, "US-ASCII"));
}

/* Determine whether a character set name refers to UTF-8. */
static int is_utf8_cs(const char *name)
{
    return name && (!strcasecmp(name, "UTF-8") || !strcasecmp(name, "UTF8"));
}

/* Determine whether a character set name refers to ISO-8859-1. */
static int is_8859_1_cs(const char *name)
{
    return name && (!strcasecmp(name, "ISO-8859-1") ||
                    !strcasecmp(name, "ISO8859-1") ||
                    !strcasecmp(name, "LATIN1") ||
                    !strcasecmp(name, "LATIN-1"));
}

/* Determine whether a character set name refers to UTF-16. */
static int is_utf16_cs(const char *name)
{
    return name && (!strcasecmp(name, "UTF-16") || !strcasecmp(name, "UTF16"));
}

/* Decode a single UTF-8 sequence from SRC advancing pointers on success. */
static int decode_utf8(const unsigned char **src, size_t *left, unsigned *cp)
{
    if (*left == 0)
        return -2;
    unsigned char c0 = (*src)[0];
    if (c0 < 0x80) {
        *cp = c0;
        (*src)++; (*left)--;
        return 0;
    }
    if ((c0 & 0xE0) == 0xC0) {
        if (*left < 2) return -2;
        unsigned char c1 = (*src)[1];
        if ((c1 & 0xC0) != 0x80)
            return -1;
        *cp = ((c0 & 0x1F) << 6) | (c1 & 0x3F);
        if (*cp < 0x80)
            return -1;
        (*src) += 2; (*left) -= 2;
        return 0;
    }
    if ((c0 & 0xF0) == 0xE0) {
        if (*left < 3) return -2;
        unsigned char c1 = (*src)[1];
        unsigned char c2 = (*src)[2];
        if ((c1 & 0xC0) != 0x80 || (c2 & 0xC0) != 0x80)
            return -1;
        *cp = ((c0 & 0x0F) << 12) | ((c1 & 0x3F) << 6) | (c2 & 0x3F);
        if (*cp < 0x800)
            return -1;
        (*src) += 3; (*left) -= 3;
        return 0;
    }
    return -1;
}

/* Encode a Unicode codepoint to UTF-8 advancing DST on success. */
static int encode_utf8(unsigned cp, unsigned char **dst, size_t *left)
{
    if (cp < 0x80) {
        if (*left < 1) return -2;
        (*dst)[0] = cp;
        (*dst)++; (*left)--;
        return 0;
    }
    if (cp < 0x800) {
        if (*left < 2) return -2;
        (*dst)[0] = 0xC0 | (cp >> 6);
        (*dst)[1] = 0x80 | (cp & 0x3F);
        (*dst) += 2; (*left) -= 2;
        return 0;
    }
    if (cp <= 0xFFFF) {
        if (*left < 3) return -2;
        (*dst)[0] = 0xE0 | (cp >> 12);
        (*dst)[1] = 0x80 | ((cp >> 6) & 0x3F);
        (*dst)[2] = 0x80 | (cp & 0x3F);
        (*dst) += 3; (*left) -= 3;
        return 0;
    }
    return -1;
}

/* Decode a 16-bit character from little-endian UTF-16. */
static int decode_utf16(const unsigned char **src, size_t *left, unsigned *cp)
{
    if (*left < 2)
        return -2;
    unsigned c = (*src)[0] | ((*src)[1] << 8);
    if (c >= 0xD800 && c <= 0xDFFF)
        return -1;
    *cp = c;
    (*src) += 2; (*left) -= 2;
    return 0;
}

/* Encode a codepoint to little-endian UTF-16. */
static int encode_utf16(unsigned cp, unsigned char **dst, size_t *left)
{
    if (cp > 0xFFFF || (cp >= 0xD800 && cp <= 0xDFFF))
        return -1;
    if (*left < 2)
        return -2;
    (*dst)[0] = cp & 0xFF;
    (*dst)[1] = (cp >> 8) & 0xFF;
    (*dst) += 2; (*left) -= 2;
    return 0;
}

/*
 * iconv_open() - acquire a conversion descriptor. On BSD systems the
 * host iconv implementation is used when available; otherwise simple
 * ASCII and UTF-8 conversions are provided.
 */
iconv_t iconv_open(const char *tocode, const char *fromcode)
{
    if (!tocode || !fromcode) {
        errno = EINVAL;
        return (iconv_t)-1;
    }

#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    iconv_t h = host_iconv_open(tocode, fromcode);
    if (h != (iconv_t)-1) {
        iconv_cd *cd = malloc(sizeof(*cd));
        if (!cd) {
            host_iconv_close(h);
            errno = ENOMEM;
            return (iconv_t)-1;
        }
        cd->type = CD_HOST;
        cd->host = h;
        return (iconv_t)cd;
    }
#endif

    iconv_cd *cd = malloc(sizeof(*cd));
    if (!cd) {
        errno = ENOMEM;
        return (iconv_t)-1;
    }

    if (is_ascii_cs(fromcode) && is_ascii_cs(tocode))
        cd->type = CD_ASCII_ASCII;
    else if (is_ascii_cs(fromcode) && is_utf8_cs(tocode))
        cd->type = CD_ASCII_UTF8;
    else if (is_utf8_cs(fromcode) && is_ascii_cs(tocode))
        cd->type = CD_UTF8_ASCII;
    else if (is_utf8_cs(fromcode) && is_utf8_cs(tocode))
        cd->type = CD_UTF8_UTF8;
    else if (is_ascii_cs(fromcode) && is_8859_1_cs(tocode))
        cd->type = CD_ASCII_8859_1;
    else if (is_8859_1_cs(fromcode) && is_ascii_cs(tocode))
        cd->type = CD_8859_1_ASCII;
    else if (is_8859_1_cs(fromcode) && is_8859_1_cs(tocode))
        cd->type = CD_8859_1_8859_1;
    else if (is_8859_1_cs(fromcode) && is_utf8_cs(tocode))
        cd->type = CD_8859_1_UTF8;
    else if (is_utf8_cs(fromcode) && is_8859_1_cs(tocode))
        cd->type = CD_UTF8_8859_1;
    else if (is_ascii_cs(fromcode) && is_utf16_cs(tocode))
        cd->type = CD_ASCII_UTF16;
    else if (is_utf16_cs(fromcode) && is_ascii_cs(tocode))
        cd->type = CD_UTF16_ASCII;
    else if (is_utf8_cs(fromcode) && is_utf16_cs(tocode))
        cd->type = CD_UTF8_UTF16;
    else if (is_utf16_cs(fromcode) && is_utf8_cs(tocode))
        cd->type = CD_UTF16_UTF8;
    else if (is_8859_1_cs(fromcode) && is_utf16_cs(tocode))
        cd->type = CD_8859_1_UTF16;
    else if (is_utf16_cs(fromcode) && is_8859_1_cs(tocode))
        cd->type = CD_UTF16_8859_1;
    else if (is_utf16_cs(fromcode) && is_utf16_cs(tocode))
        cd->type = CD_UTF16_UTF16;
    else {
        free(cd);
        errno = EINVAL;
        return (iconv_t)-1;
    }

    return (iconv_t)cd;
}

/*
 * iconv() - convert a sequence of characters according to the
 * conversion descriptor. Supports basic ASCII and UTF-8 handling
 * and forwards to the host iconv implementation when present.
 */
size_t iconv(iconv_t cd_, char **inbuf, size_t *inbytesleft,
             char **outbuf, size_t *outbytesleft)
{
    iconv_cd *cd = (iconv_cd *)cd_;
    if (!cd) {
        errno = EINVAL;
        return (size_t)-1;
    }

#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    if (cd->type == CD_HOST)
        return host_iconv(cd->host, inbuf, inbytesleft, outbuf, outbytesleft);
#endif

    char *src = inbuf ? *inbuf : NULL;
    char *dst = outbuf ? *outbuf : NULL;
    size_t inleft = inbytesleft ? *inbytesleft : 0;
    size_t outleft = outbytesleft ? *outbytesleft : 0;
    size_t converted = 0;

    while (inleft > 0) {
        unsigned char c = (unsigned char)*src;
        unsigned cp = 0;
        int r;

        switch (cd->type) {
        case CD_ASCII_ASCII:
        case CD_ASCII_UTF8:
        case CD_ASCII_8859_1:
            if (c >= 0x80) { errno = EILSEQ; return (size_t)-1; }
            if (outleft < 1) { errno = E2BIG; return (size_t)-1; }
            *dst++ = *src++; inleft--; outleft--; converted++; break;

        case CD_8859_1_ASCII:
            if (c >= 0x80) { errno = EILSEQ; return (size_t)-1; }
            if (outleft < 1) { errno = E2BIG; return (size_t)-1; }
            *dst++ = *src++; inleft--; outleft--; converted++; break;

        case CD_8859_1_8859_1:
            if (outleft < 1) { errno = E2BIG; return (size_t)-1; }
            *dst++ = *src++; inleft--; outleft--; converted++; break;

        case CD_8859_1_UTF8:
            if (c < 0x80) {
                if (outleft < 1) { errno = E2BIG; return (size_t)-1; }
                *dst++ = *src++; inleft--; outleft--; converted++; break;
            } else {
                if (outleft < 2) { errno = E2BIG; return (size_t)-1; }
                dst[0] = 0xC0 | (c >> 6);
                dst[1] = 0x80 | (c & 0x3F);
                dst += 2; src++; inleft--; outleft -= 2; converted++; break;
            }

        case CD_UTF8_ASCII:
            r = decode_utf8((const unsigned char **)&src, &inleft, &cp);
            if (r == -2) { errno = EINVAL; return (size_t)-1; }
            if (r == -1 || cp > 0x7F) { errno = EILSEQ; return (size_t)-1; }
            if (outleft < 1) { errno = E2BIG; return (size_t)-1; }
            *dst++ = (char)cp; outleft--; converted++; break;

        case CD_UTF8_8859_1:
            r = decode_utf8((const unsigned char **)&src, &inleft, &cp);
            if (r == -2) { errno = EINVAL; return (size_t)-1; }
            if (r == -1 || cp > 0xFF) { errno = EILSEQ; return (size_t)-1; }
            if (outleft < 1) { errno = E2BIG; return (size_t)-1; }
            *dst++ = (char)cp; outleft--; converted++; break;

        case CD_UTF8_UTF8:
            if (outleft < 1) { errno = E2BIG; return (size_t)-1; }
            *dst++ = *src++; inleft--; outleft--; converted++; break;

        case CD_ASCII_UTF16:
            if (c >= 0x80) { errno = EILSEQ; return (size_t)-1; }
            r = encode_utf16(c, (unsigned char **)&dst, &outleft);
            if (r == -2) { errno = E2BIG; return (size_t)-1; }
            src++; inleft--; converted++; break;

        case CD_UTF16_ASCII:
            r = decode_utf16((const unsigned char **)&src, &inleft, &cp);
            if (r == -2) { errno = EINVAL; return (size_t)-1; }
            if (r == -1 || cp > 0x7F) { errno = EILSEQ; return (size_t)-1; }
            if (outleft < 1) { errno = E2BIG; return (size_t)-1; }
            *dst++ = (char)cp; outleft--; converted++; break;

        case CD_UTF8_UTF16:
            r = decode_utf8((const unsigned char **)&src, &inleft, &cp);
            if (r == -2) { errno = EINVAL; return (size_t)-1; }
            if (r == -1) { errno = EILSEQ; return (size_t)-1; }
            r = encode_utf16(cp, (unsigned char **)&dst, &outleft);
            if (r == -1) { errno = EILSEQ; return (size_t)-1; }
            if (r == -2) { errno = E2BIG; return (size_t)-1; }
            converted++; break;

        case CD_UTF16_UTF8:
            r = decode_utf16((const unsigned char **)&src, &inleft, &cp);
            if (r == -2) { errno = EINVAL; return (size_t)-1; }
            if (r == -1) { errno = EILSEQ; return (size_t)-1; }
            r = encode_utf8(cp, (unsigned char **)&dst, &outleft);
            if (r == -1) { errno = EILSEQ; return (size_t)-1; }
            if (r == -2) { errno = E2BIG; return (size_t)-1; }
            converted++; break;

        case CD_8859_1_UTF16:
            r = encode_utf16(c, (unsigned char **)&dst, &outleft);
            if (r == -2) { errno = E2BIG; return (size_t)-1; }
            src++; inleft--; converted++; break;

        case CD_UTF16_8859_1:
            r = decode_utf16((const unsigned char **)&src, &inleft, &cp);
            if (r == -2) { errno = EINVAL; return (size_t)-1; }
            if (r == -1 || cp > 0xFF) { errno = EILSEQ; return (size_t)-1; }
            if (outleft < 1) { errno = E2BIG; return (size_t)-1; }
            *dst++ = (char)cp; outleft--; converted++; break;

        case CD_UTF16_UTF16:
            if (inleft < 2) { errno = EINVAL; return (size_t)-1; }
            if (outleft < 2) { errno = E2BIG; return (size_t)-1; }
            dst[0] = src[0]; dst[1] = src[1];
            dst += 2; src += 2;
            inleft -= 2; outleft -= 2; converted++; break;

        default:
            errno = EINVAL; return (size_t)-1;
        }
    }

    if (inbuf)
        *inbuf = src;
    if (inbytesleft)
        *inbytesleft = inleft;
    if (outbuf)
        *outbuf = dst;
    if (outbytesleft)
        *outbytesleft = outleft;

    return converted;
}

/*
 * iconv_close() - release a conversion descriptor obtained from
 * iconv_open(). Forwards to the host implementation on BSD when
 * necessary.
 */
int iconv_close(iconv_t cd_)
{
    iconv_cd *cd = (iconv_cd *)cd_;
    if (!cd)
        return -1;

#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    if (cd->type == CD_HOST) {
        int r = host_iconv_close(cd->host);
        free(cd);
        return r;
    }
#endif

    free(cd);
    return 0;
}

