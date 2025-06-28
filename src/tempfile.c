/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms, with or without modification, are permitted provided that the copyright notice and this permission notice appear in all copies. This software is provided "as is" without warranty.
 *
 * Purpose: Implements the tempfile functions for vlibc. Provides wrappers and helpers used by the standard library.
 *
 * Copyright (c) 2025
 */

#include "stdio.h"
#include "io.h"
#include "memory.h"
#include "string.h"
#include "errno.h"
#include "stdlib.h"
#include <fcntl.h>
#include <unistd.h>

static int fill_random(unsigned char *buf, size_t len)
{
    arc4random_buf(buf, len);
    return 0;
}

static int replace_x(char *template)
{
    static const char chars[] =
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    size_t len = strlen(template);
    if (len < 6 || strcmp(template + len - 6, "XXXXXX") != 0) {
        errno = EINVAL;
        return -1;
    }
    unsigned char rnd[6];
    if (fill_random(rnd, sizeof(rnd)) < 0)
        return -1;
    for (int i = 0; i < 6; i++)
        template[len - 6 + i] = chars[rnd[i] % (sizeof(chars) - 1)];
    return 0;
}

/*
 * mkstemp() - create and open a unique temporary file from 'template'.
 * The last six X characters are replaced with random data.  Returns a
 * file descriptor opened for read/write or -1 on error.
 */
int mkstemp(char *template)
{
    if (!template) {
        errno = EINVAL;
        return -1;
    }
    for (int i = 0; i < 100; i++) {
        if (replace_x(template) < 0)
            return -1;
        int fd = open(template, O_RDWR | O_CREAT | O_EXCL, 0600);
        if (fd >= 0)
            return fd;
        if (errno != EEXIST)
            return -1;
    }
    errno = EEXIST;
    return -1;
}

/*
 * mkdtemp() - create a unique temporary directory from 'template'.  The
 * pattern must end with six X characters which are replaced in place.
 * Returns the resulting path on success or NULL on failure.
 */
char *mkdtemp(char *template)
{
    if (!template) {
        errno = EINVAL;
        return NULL;
    }
    for (int i = 0; i < 100; i++) {
        if (replace_x(template) < 0)
            return NULL;
        if (mkdir(template, 0700) == 0)
            return template;
        if (errno != EEXIST)
            return NULL;
    }
    errno = EEXIST;
    return NULL;
}

/*
 * tmpfile() - create a temporary file that is automatically removed
 * when closed.  The file is opened in read/write mode and unlinked
 * immediately after creation.
 */
FILE *tmpfile(void)
{
    char tmpl[] = "/tmp/vlibcXXXXXX";
    int fd = mkstemp(tmpl);
    if (fd < 0)
        return NULL;
    unlink(tmpl);
    FILE *f = malloc(sizeof(FILE));
    if (!f) {
        close(fd);
        errno = ENOMEM;
        return NULL;
    }
    memset(f, 0, sizeof(FILE));
    atomic_flag_clear(&f->lock);
    f->fd = fd;
    return f;
}

/*
 * tmpnam() - generate a unique temporary filename.  If 's' is NULL a
 * static buffer is used, otherwise the result is written to 's'.  The
 * file is not created.
 */
char *tmpnam(char *s)
{
    static char buf[L_tmpnam];
    if (s) {
        size_t sz = __builtin_object_size(s, 0);
        if (sz != (size_t)-1 && sz < L_tmpnam) {
            errno = ERANGE;
            return NULL;
        }
    }
    char *out = s ? s : buf;
    strcpy(out, "/tmp/vlibcXXXXXX");
    int fd = mkstemp(out);
    if (fd < 0)
        return NULL;
    close(fd);
    unlink(out);
    return out;
}

/*
 * tempnam() - return a pathname for a temporary file.  The directory
 * and prefix can be specified with 'dir' and 'pfx'.  The name is
 * allocated with malloc and should be freed by the caller.  The file
 * itself is not created.
 */
char *tempnam(const char *dir, const char *pfx)
{
    const char *d = dir ? dir : "/tmp";
    const char *p = pfx ? pfx : "vlibc";
    char tmpl[256];
    snprintf(tmpl, sizeof(tmpl), "%s/%sXXXXXX", d, p);
    char *res = strdup(tmpl);
    if (!res)
        return NULL;
    int fd = mkstemp(res);
    if (fd < 0) {
        free(res);
        return NULL;
    }
    close(fd);
    unlink(res);
    return res;
}
