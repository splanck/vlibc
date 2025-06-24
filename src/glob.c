/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms, with or without modification, are permitted provided that the copyright notice and this permission notice appear in all copies. This software is provided "as is" without warranty.
 *
 * Purpose: Implements the glob functions for vlibc. Provides wrappers and helpers used by the standard library.
 *
 * Copyright (c) 2025
 */

#include "glob.h"
#include "string.h"
#include "memory.h"
#include "stdlib.h"
#include "dirent.h"
#include "fnmatch.h"
#include <errno.h>

static char *join_path(const char *base, const char *name)
{
    size_t blen = base ? strlen(base) : 0;
    size_t add = (blen && base[blen-1] != '/') ? 1 : 0;
    size_t nlen = strlen(name);
    char *res = malloc(blen + add + nlen + 1);
    if (!res)
        return NULL;
    if (blen) {
        memcpy(res, base, blen);
        if (add)
            res[blen] = '/';
    }
    size_t pos = blen + add;
    memcpy(res + pos, name, nlen);
    res[pos + nlen] = '\0';
    return res;
}

static int add_match(glob_t *g, const char *path)
{
    char **tmp = realloc(g->gl_pathv, sizeof(char*) * (g->gl_pathc + 2));
    if (!tmp)
        return -1;
    g->gl_pathv = tmp;
    g->gl_pathv[g->gl_pathc] = strdup(path);
    if (!g->gl_pathv[g->gl_pathc])
        return -1;
    g->gl_pathc++;
    g->gl_pathv[g->gl_pathc] = NULL;
    return 0;
}

static int cmp_str(const void *a, const void *b)
{
    const char *sa = *(const char *const*)a;
    const char *sb = *(const char *const*)b;
    return strcmp(sa, sb);
}

static int expand(const char *base, const char *pattern, glob_t *g, int flags,
                  int (*errfunc)(const char *, int));

static int handle_dir(const char *dir, const char *pattern, const char *rest,
                      glob_t *g, int flags,
                      int (*errfunc)(const char *, int))
{
    DIR *d = opendir(*dir ? dir : ".");
    if (!d) {
        if (errfunc)
            errfunc(dir, errno);
        return 0;
    }
    struct dirent *ent;
    int ret = 0;
    int fnm_flags = (flags & GLOB_NOESCAPE) ? FNM_NOESCAPE : 0;
    while ((ent = readdir(d))) {
        const char *name = ent->d_name;
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
            continue;
        if (fnmatch(pattern, name, fnm_flags) == 0) {
            char *next = join_path(dir, name);
            if (!next) { ret = GLOB_NOSPACE; break; }
            if (rest) {
                ret = expand(next, rest, g, flags, errfunc);
            } else {
                if (add_match(g, next) < 0)
                    ret = GLOB_NOSPACE;
            }
            free(next);
            if (ret)
                break;
        }
    }
    closedir(d);
    return ret;
}

static int expand(const char *base, const char *pattern, glob_t *g, int flags,
                  int (*errfunc)(const char *, int))
{
    while (*pattern == '/')
        pattern++;
    const char *slash = strchr(pattern, '/');
    if (!slash)
        return handle_dir(base, pattern, NULL, g, flags, errfunc);

    size_t len = (size_t)(slash - pattern);
    char part[len + 1];
    memcpy(part, pattern, len);
    part[len] = '\0';
    const char *rest = slash[1] ? slash + 1 : NULL;

    return handle_dir(base, part, rest, g, flags, errfunc);
}

int glob(const char *pattern, int flags,
         int (*errfunc)(const char *epath, int eerrno), glob_t *pglob)
{
    if (!pglob)
        return GLOB_ABORTED;
    pglob->gl_pathc = 0;
    pglob->gl_pathv = NULL;
    pglob->gl_offs = 0;

    if (!pattern)
        return GLOB_ABORTED;

    int ret;
    if (pattern[0] == '/')
        ret = expand("/", pattern + 1, pglob, flags, errfunc);
    else
        ret = expand("", pattern, pglob, flags, errfunc);

    if (ret == 0 && pglob->gl_pathc == 0)
        ret = GLOB_NOMATCH;
    if (ret == 0 && !(flags & GLOB_NOSORT))
        qsort(pglob->gl_pathv, pglob->gl_pathc, sizeof(char*), cmp_str);
    return ret;
}

void globfree(glob_t *pglob)
{
    if (!pglob)
        return;
    for (size_t i = 0; i < pglob->gl_pathc; i++)
        free(pglob->gl_pathv[i]);
    free(pglob->gl_pathv);
    pglob->gl_pathv = NULL;
    pglob->gl_pathc = 0;
    pglob->gl_offs = 0;
}

