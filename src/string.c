#include "string.h"
#include "memory.h"

size_t vstrlen(const char *s)
{
    const char *p = s;
    while (*p)
        p++;
    return (size_t)(p - s);
}

size_t strnlen(const char *s, size_t maxlen)
{
    const char *p = s;
    while (maxlen-- && *p)
        p++;
    return (size_t)(p - s);
}

char *vstrcpy(char *dest, const char *src)
{
    char *d = dest;
    while (*src) {
        *d++ = *src++;
    }
    *d = '\0';
    return dest;
}

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

int strcmp(const char *s1, const char *s2)
{
    return vstrncmp(s1, s2, (size_t)-1);
}

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

char *strdup(const char *s)
{
    size_t len = vstrlen(s);
    char *dup = malloc(len + 1);
    if (!dup)
        return NULL;
    vstrcpy(dup, s);
    return dup;
}

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

static char *strtok_static;

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

char *strtok(char *str, const char *delim)
{
    return strtok_r(str, delim, &strtok_static);
}
