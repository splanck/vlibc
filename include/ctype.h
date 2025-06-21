#ifndef CTYPE_H
#define CTYPE_H

/* Character classification bit masks */
#define __CTYPE_UPPER  0x01
#define __CTYPE_LOWER  0x02
#define __CTYPE_DIGIT  0x04
#define __CTYPE_SPACE  0x08
#define __CTYPE_XDIGIT 0x10

/* Macro helpers */
#define __ctype_isflag(c, flag) \
    (((unsigned char)(c)) < 128 && ((__ctype_table[(unsigned char)(c)] & (flag)) != 0))

/* Classification table defined in ctype.c */
extern const unsigned char __ctype_table[128];

#define isalpha(c) (__ctype_isflag(c, __CTYPE_UPPER | __CTYPE_LOWER))
#define isdigit(c) (__ctype_isflag(c, __CTYPE_DIGIT))
#define isalnum(c) (__ctype_isflag(c, __CTYPE_UPPER | __CTYPE_LOWER | __CTYPE_DIGIT))
#define isspace(c) (__ctype_isflag(c, __CTYPE_SPACE))
#define isupper(c) (__ctype_isflag(c, __CTYPE_UPPER))
#define islower(c) (__ctype_isflag(c, __CTYPE_LOWER))
#define isxdigit(c) (__ctype_isflag(c, __CTYPE_XDIGIT))

#define tolower(c) (isupper(c) ? ((c) + ('a' - 'A')) : (c))
#define toupper(c) (islower(c) ? ((c) + ('A' - 'a')) : (c))

#endif /* CTYPE_H */
