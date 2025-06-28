/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms,
 * with or without modification, are permitted provided that the copyright
 * notice and this permission notice appear in all copies. This software is
 * provided "as is" without warranty.
 *
 * Purpose: Implements basic visual encoding helpers for vlibc.
 */

#include "vis.h"
#include "string.h"
#include <ctype.h>
#include <stdio.h>

/* Return non-zero if character requires octal escaping */
static int need_octal(int c, int flag)
{
    if (flag & VIS_OCTAL)
        return 1;
    if (!isprint(c))
        return 1;
    if ((c == ' ' && (flag & (VIS_SP|VIS_WHITE))) ||
        (c == '\t' && (flag & (VIS_TAB|VIS_WHITE))) ||
        (c == '\n' && (flag & (VIS_NL|VIS_WHITE))))
        return 1;
    return 0;
}

/*
 * nvis() - encode a character using visual escapes.
 * Writes the representation of 'c' into 'dst' respecting
 * the destination length and returns the number of bytes
 * written or -1 on overflow.
 */
int nvis(char *dst, size_t dlen, int c, int flag, int nextc)
{
    (void)nextc; /* unused */
    unsigned char ch = (unsigned char)c;
    char buf[5];
    size_t len = 0;

    if (!need_octal(ch, flag)) {
        if (ch == '\\' && !(flag & VIS_NOSLASH)) {
            buf[0] = '\\'; buf[1] = '\\'; buf[2] = '\0'; len = 2;
        } else {
            buf[0] = ch; buf[1] = '\0'; len = 1;
        }
    } else if ((flag & VIS_CSTYLE) != 0) {
        switch (ch) {
        case '\n': buf[0] = '\\'; buf[1] = 'n'; buf[2] = '\0'; len = 2; break;
        case '\r': buf[0] = '\\'; buf[1] = 'r'; buf[2] = '\0'; len = 2; break;
        case '\b': buf[0] = '\\'; buf[1] = 'b'; buf[2] = '\0'; len = 2; break;
        case '\a': buf[0] = '\\'; buf[1] = 'a'; buf[2] = '\0'; len = 2; break;
        case '\v': buf[0] = '\\'; buf[1] = 'v'; buf[2] = '\0'; len = 2; break;
        case '\t': buf[0] = '\\'; buf[1] = 't'; buf[2] = '\0'; len = 2; break;
        case '\f': buf[0] = '\\'; buf[1] = 'f'; buf[2] = '\0'; len = 2; break;
        case '\\': buf[0] = '\\'; buf[1] = '\\'; buf[2] = '\0'; len = 2; break;
        default:
            snprintf(buf, sizeof(buf), "\\%03o", ch);
            len = strlen(buf);
            break;
        }
    } else {
        snprintf(buf, sizeof(buf), (flag & VIS_NOSLASH)?"%03o":"\\%03o", ch);
        len = strlen(buf);
    }

    if (len >= dlen)
        return -1;
    memcpy(dst, buf, len + 1);
    return (int)len;
}

/*
 * vis() - convenience wrapper around nvis() without a
 * destination length limit.
 */
int vis(char *dst, int c, int flag, int nextc)
{
    (void)nextc;
    return nvis(dst, (size_t)-1, c, flag, 0);
}

/* simple state machine for decoding */
#define S_GROUND 0
#define S_ESCAPE 1
#define S_OCT1   2
#define S_OCT2   3

/*
 * unvis() - decode a byte stream produced by vis.
 * Maintains state across calls and returns UNVIS_* codes
 * describing the result of each step.
 */
int unvis(char *cp, int c, int *state, int flag)
{
    unsigned char ch = (unsigned char)c;
    int st = *state & 0xff;
    int val = (*state >> 8) & 0xff;
    (void)flag;

    switch (st) {
    case S_GROUND:
        if (ch == '\\') {
            *state = S_ESCAPE;
            return UNVIS_NOCHAR;
        }
        *cp = ch;
        return UNVIS_VALID;

    case S_ESCAPE:
        if (ch >= '0' && ch <= '7') {
            val = ch - '0';
            *state = (val << 8) | S_OCT1;
            return UNVIS_NOCHAR;
        }
        *state = S_GROUND;
        switch (ch) {
        case 'n': *cp = '\n'; return UNVIS_VALID;
        case 'r': *cp = '\r'; return UNVIS_VALID;
        case 'b': *cp = '\b'; return UNVIS_VALID;
        case 'a': *cp = '\a'; return UNVIS_VALID;
        case 'v': *cp = '\v'; return UNVIS_VALID;
        case 't': *cp = '\t'; return UNVIS_VALID;
        case 'f': *cp = '\f'; return UNVIS_VALID;
        case '\\': *cp = '\\'; return UNVIS_VALID;
        default:
            *cp = ch;
            return UNVIS_VALID;
        }

    case S_OCT1:
        if (ch >= '0' && ch <= '7') {
            val = (val << 3) + (ch - '0');
            *state = (val << 8) | S_OCT2;
            return UNVIS_NOCHAR;
        }
        /* FALLTHROUGH */
    case S_OCT2:
        *state = S_GROUND;
        *cp = val;
        return UNVIS_VALID;
    }

    *state = S_GROUND;
    return UNVIS_SYNBAD;
}
