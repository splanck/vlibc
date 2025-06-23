#include "fnmatch.h"
#include "string.h"

static int match_range(const char *p, char c, int flags, const char **end)
{
    int negate = 0;
    int match = 0;
    size_t i = 1;

    if (p[i] == '!' || p[i] == '^') {
        negate = 1;
        i++;
    }

    for (; p[i]; i++) {
        char pc = p[i];
        if (pc == ']' && i > 1 + negate)
            break;
        if (pc == '\\' && !(flags & FNM_NOESCAPE)) {
            i++;
            pc = p[i];
        }
        if (p[i + 1] == '-' && p[i + 2] && p[i + 2] != ']') {
            char start = pc;
            i += 2;
            char endc = p[i];
            if (endc == '\\' && !(flags & FNM_NOESCAPE)) {
                i++;
                endc = p[i];
            }
            if (c >= start && c <= endc)
                match = 1;
        } else {
            if (c == pc)
                match = 1;
        }
    }

    if (p[i] != ']') {
        *end = p + 1;
        return (c == '[');
    }
    *end = p + i + 1;
    return negate ? !match : match;
}

int fnmatch(const char *pattern, const char *string, int flags)
{
    const char *p = pattern;
    const char *s = string;

    while (*p) {
        if (*p == '?') {
            if (*s == '\0')
                return FNM_NOMATCH;
            s++; p++;
        } else if (*p == '*') {
            while (*p == '*')
                p++;
            if (*p == '\0')
                return 0;
            while (*s) {
                int r = fnmatch(p, s, flags);
                if (r == 0)
                    return 0;
                s++;
            }
            return FNM_NOMATCH;
        } else if (*p == '[') {
            if (*s == '\0')
                return FNM_NOMATCH;
            const char *next;
            if (!match_range(p, *s, flags, &next))
                return FNM_NOMATCH;
            s++; p = next;
        } else if (*p == '\\' && !(flags & FNM_NOESCAPE)) {
            p++;
            if (*p == '\0')
                return FNM_NOMATCH;
            if (*p != *s)
                return FNM_NOMATCH;
            s++; p++;
        } else {
            if (*p != *s)
                return FNM_NOMATCH;
            s++; p++;
        }
    }
    return *s ? FNM_NOMATCH : 0;
}

