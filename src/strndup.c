#include "string.h"
#include "memory.h"

char *strndup(const char *s, size_t n)
{
    size_t len = strnlen(s, n);
    char *dup = malloc(len + 1);
    if (!dup)
        return NULL;
    vmemcpy(dup, s, len);
    dup[len] = '\0';
    return dup;
}
