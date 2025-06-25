/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms, with or without modification, are permitted provided that the copyright notice and this permission notice appear in all copies. This software is provided "as is" without warranty.
 *
 * Purpose: Implements the string functions for vlibc. Provides wrappers and helpers used by the standard library.
 *
 * Copyright (c) 2025
 */

#include "string.h"
#include "memory.h"

/* Return the length of a NUL terminated string. */
size_t vstrlen(const char *s)
{
    const char *p = s;
    while (*p)
        p++;
    return (size_t)(p - s);
}

/* Like strlen but stops after maxlen characters. */
size_t strnlen(const char *s, size_t maxlen)
{
    const char *p = s;
    while (maxlen-- && *p)
        p++;
    return (size_t)(p - s);
}

/* Copy src string to dest, returning dest. */
char *vstrcpy(char *dest, const char *src)
{
    char *d = dest;
    while (*src) {
        *d++ = *src++;
    }
    *d = '\0';
    return dest;
}

/* Compare two strings up to n characters. */
int vstrncmp(const char *s1, const char *s2, size_t n)
{
    while (n--) {
        unsigned char c1 = (unsigned char)*s1++;
        unsigned char c2 = (unsigned char)*s2++;
        if (c1 != c2)
            return c1 - c2;
        if (c1 == '\0')
            break;
    }
    return 0;
}

/* Wrapper around vstrncmp for complete comparison. */
int strcmp(const char *s1, const char *s2)
{
    return vstrncmp(s1, s2, (size_t)-1);
}

/* Find the first occurrence of c in s. */
char *strchr(const char *s, int c)
{
    unsigned char ch = (unsigned char)c;
    while (*s) {
        if ((unsigned char)*s == ch)
            return (char *)s;
        s++;
    }
    if (ch == '\0')
        return (char *)s;
    return NULL;
}

/* Allocate a duplicate of s using malloc. */
char *strdup(const char *s)
{
    size_t len = vstrlen(s);
    char *dup = malloc(len + 1);
    if (!dup)
        return NULL;
    vstrcpy(dup, s);
    return dup;
}

/* Bounded string copy that NUL pads the result. */
char *strncpy(char *dest, const char *src, size_t n)
{
    char *d = dest;
    while (n && *src) {
        *d++ = *src++;
        --n;
    }
    while (n--) {
        *d++ = '\0';
    }
    return dest;
}

/* Append src to dest. */
char *strcat(char *dest, const char *src)
{
    char *d = dest;
    while (*d)
        d++;
    while (*src)
        *d++ = *src++;
    *d = '\0';
    return dest;
}

/* Append at most n characters from src to dest. */
char *strncat(char *dest, const char *src, size_t n)
{
    char *d = dest;
    while (*d)
        d++;
    while (n-- && *src)
        *d++ = *src++;
    *d = '\0';
    return dest;
}

/* Fallback storage for strtok when strtok_r is not used. */
static char *strtok_static;

/* Reentrant tokenizer splitting a string using delimiters. */
char *strtok_r(char *str, const char *delim, char **saveptr)
{
    char *s;

    if (str)
        s = str;
    else if (saveptr && *saveptr)
        s = *saveptr;
    else
        return NULL;

    /* skip leading delimiters */
    while (*s && strchr(delim, *s))
        s++;

    if (*s == '\0') {
        if (saveptr)
            *saveptr = NULL;
        return NULL;
    }

    char *token = s;

    while (*s && !strchr(delim, *s))
        s++;

    if (*s) {
        *s = '\0';
        s++;
        if (saveptr)
            *saveptr = s;
    } else if (saveptr) {
        *saveptr = NULL;
    }

    return token;
}

/* Non-reentrant tokenizer using a static state. */
char *strtok(char *str, const char *delim)
{
    return strtok_r(str, delim, &strtok_static);
}
