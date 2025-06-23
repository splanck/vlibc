#include "fnmatch.h"
#include "string.h"

static int match_range(const char *p, char c, int flags, const char **ep)
{
    int neg = 0;
    int ok = 0;
    if (*p == '!' || *p == '^') {
        neg = 1;
        p++;
    }
    while (*p && *p != ']') {
        char start;
        if (*p == '\\' && !(flags & FNM_NOESCAPE) && p[1]) {
            start = p[1];
            p += 2;
        } else {
            start = *p++;
        }
        if (*p == '-' && p[1] && p[1] != ']') {
            p++;
            char end;
            if (*p == '\\' && !(flags & FNM_NOESCAPE) && p[1]) {
                end = p[1];
                p += 2;
            } else {
                end = *p++;
            }
            if (start <= c && c <= end)
                ok = 1;
        } else {
            if (start == c)
                ok = 1;
        }
    }
    if (*p == ']')
        p++;
    *ep = p;
    return neg ? !ok : ok;
}

static int do_match(const char *p, const char *s, int flags)
{
    while (*p) {
        char pc = *p++;
        if (pc == '?') {
            if (!*s)
                return 0;
            s++;
            continue;
        } else if (pc == '*') {
            while (*p == '*')
                p++;
            if (!*p)
                return 1;
            while (*s) {
                if (do_match(p, s, flags))
                    return 1;
                s++;
            }
            return 0;
        } else if (pc == '[') {
            if (!*s)
                return 0;
            const char *end = p;
            if (!match_range(end, *s, flags, &end))
                return 0;
            p = end;
            s++;
            continue;
        } else if (pc == '\\' && !(flags & FNM_NOESCAPE)) {
            pc = *p++;
            if (!pc)
                pc = '\\';
        }
        if (*s != pc)
            return 0;
        s++;
    }
    return *s == '\0';
}

int fnmatch(const char *pattern, const char *string, int flags)
{
    return do_match(pattern, string, flags) ? 0 : FNM_NOMATCH;
}

