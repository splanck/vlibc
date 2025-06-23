#include "stdlib.h"
#include "string.h"
#include "memory.h"

char *basename(const char *path)
{
    if (!path || !*path)
        return strdup(".");

    const char *start = path;
    const char *end = path + strlen(path);
    while (end > start && *(end - 1) == '/')
        --end;
    if (end == start)
        return strdup("/");

    const char *p = end;
    while (p > start && *(p - 1) != '/')
        --p;

    size_t len = end - p;
    char *out = malloc(len + 1);
    if (!out)
        return NULL;
    memcpy(out, p, len);
    out[len] = '\0';
    return out;
}

char *dirname(const char *path)
{
    if (!path || !*path)
        return strdup(".");

    const char *start = path;
    const char *end = path + strlen(path);
    while (end > start && *(end - 1) == '/')
        --end;
    if (end == start)
        return strdup("/");

    const char *p = end;
    while (p > start && *(p - 1) != '/')
        --p;
    while (p > start && *(p - 1) == '/')
        --p;

    size_t len = p - start;
    if (len == 0)
        return strdup(".");

    char *out = malloc(len + 1);
    if (!out)
        return NULL;
    memcpy(out, start, len);
    out[len] = '\0';
    return out;
}

