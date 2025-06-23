#include "string.h"
#include "ctype.h"

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

int strncasecmp(const char *s1, const char *s2, size_t n)
{
    while (n--) {
        unsigned char c1 = (unsigned char)*s1++;
        unsigned char c2 = (unsigned char)*s2++;
        int diff = tolower(c1) - tolower(c2);
        if (diff)
            return diff;
        if (c1 == '\0')
            break;
    }
    return 0;
}

int strcasecmp(const char *s1, const char *s2)
{
    return strncasecmp(s1, s2, (size_t)-1);
}

size_t strlcpy(char *dst, const char *src, size_t size)
{
    size_t len = vstrlen(src);
    if (size) {
        size_t copy = len >= size ? size - 1 : len;
        vmemcpy(dst, src, copy);
        dst[copy] = '\0';
    }
    return len;
}

size_t strlcat(char *dst, const char *src, size_t size)
{
    size_t dlen = vstrlen(dst);
    size_t slen = vstrlen(src);

    if (dlen < size) {
        size_t copy = slen >= size - dlen ? size - dlen - 1 : slen;
        vmemcpy(dst + dlen, src, copy);
        dst[dlen + copy] = '\0';
    }

    return dlen + slen;
}

size_t strspn(const char *s, const char *accept)
{
    size_t count = 0;
    while (*s) {
        const char *a = accept;
        int found = 0;
        while (*a) {
            if (*s == *a) {
                found = 1;
                break;
            }
            a++;
        }
        if (!found)
            break;
        ++count;
        ++s;
    }
    return count;
}

size_t strcspn(const char *s, const char *reject)
{
    size_t count = 0;
    while (*s) {
        const char *r = reject;
        int stop = 0;
        while (*r) {
            if (*s == *r) {
                stop = 1;
                break;
            }
            r++;
        }
        if (stop)
            break;
        ++count;
        ++s;
    }
    return count;
}

char *strpbrk(const char *s, const char *accept)
{
    while (*s) {
        const char *a = accept;
        while (*a) {
            if (*s == *a)
                return (char *)s;
            a++;
        }
        s++;
    }
    return NULL;
}
