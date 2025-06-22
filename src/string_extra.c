#include "string.h"

void *memchr(const void *s, int c, size_t n)
{
    const unsigned char *p = s;
    unsigned char ch = (unsigned char)c;
    while (n--) {
        if (*p == ch)
            return (void *)p;
        p++;
    }
    return NULL;
}

char *strrchr(const char *s, int c)
{
    const char *ret = NULL;
    unsigned char ch = (unsigned char)c;
    while (1) {
        if (*s == ch)
            ret = s;
        if (*s == '\0')
            break;
        s++;
    }
    return (char *)ret;
}

char *strstr(const char *haystack, const char *needle)
{
    if (!*needle)
        return (char *)haystack;
    size_t nlen = vstrlen(needle);
    while (*haystack) {
        if (*haystack == *needle && vstrncmp(haystack, needle, nlen) == 0)
            return (char *)haystack;
        haystack++;
    }
    return NULL;
}
