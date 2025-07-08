/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms, with or without modification, are permitted provided that the copyright notice and this permission notice appear in all copies. This software is provided "as is" without warranty.
 *
 * Purpose: Implements the pty functions for vlibc. Provides wrappers and helpers used by the standard library.
 *
 * Copyright (c) 2025
 */

#include "pty.h"
#include "io.h"
#include "process.h"
#include "unistd.h"
#include "errno.h"
#include "string.h"
#include "stdlib.h"
#include <fcntl.h>
#include <limits.h>

#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
extern int host_openpty(int *, int *, char *, struct termios *, struct winsize *) __asm("openpty");
extern int host_forkpty(int *, char *, struct termios *, struct winsize *) __asm("forkpty");

int openpty(int *amaster, int *aslave, char *name, size_t namesz,
            struct termios *termp, struct winsize *winp)
{
    if (!amaster || !aslave)
        return (errno = EINVAL, -1);
    char *buf = NULL;
    size_t cap = 0;
    if (name) {
#ifdef PATH_MAX
        cap = PATH_MAX;
#else
        cap = 256;
#endif
        buf = malloc(cap);
        if (!buf)
            return (errno = ENOMEM, -1);
    }
    int r = host_openpty(amaster, aslave, name ? buf : NULL, termp, winp);
    if (r == 0 && name)
        strlcpy(name, buf, namesz);
    free(buf);
    return r;
}

int forkpty(int *amaster, char *name, size_t namesz,
            struct termios *termp, struct winsize *winp)
{
    char *buf = NULL;
    size_t cap = 0;
    if (name) {
#ifdef PATH_MAX
        cap = PATH_MAX;
#else
        cap = 256;
#endif
        buf = malloc(cap);
        if (!buf)
            return (errno = ENOMEM, -1);
    }
    int pid = host_forkpty(amaster, name ? buf : NULL, termp, winp);
    if (pid >= 0 && name)
        strlcpy(name, buf, namesz);
    free(buf);
    return pid;
}

#else

extern int host_posix_openpt(int) __asm("posix_openpt");
extern int host_grantpt(int) __asm("grantpt");
extern int host_unlockpt(int) __asm("unlockpt");
extern int host_ptsname_r(int, char *, size_t) __asm("ptsname_r");

static int do_openpty(int *amaster, int *aslave, char *name, size_t namesz,
                      struct termios *termp, struct winsize *winp)
{
    int m = host_posix_openpt(O_RDWR | O_NOCTTY);
    if (m < 0)
        return -1;
    if (host_grantpt(m) < 0 || host_unlockpt(m) < 0) {
        close(m);
        return -1;
    }
    size_t cap = 0;
#ifdef PATH_MAX
    cap = PATH_MAX;
#else
    cap = 256;
#endif
    char *buf = malloc(cap);
    if (!buf) {
        close(m);
        errno = ENOMEM;
        return -1;
    }
    for (;;) {
        if (host_ptsname_r(m, buf, cap) == 0)
            break;
        if (errno != ERANGE) {
            free(buf);
            close(m);
            return -1;
        }
        if (cap > SIZE_MAX / 2) {
            free(buf);
            close(m);
            errno = ENAMETOOLONG;
            return -1;
        }
        size_t new_cap = cap * 2;
        char *tmp = realloc(buf, new_cap);
        if (!tmp) {
            free(buf);
            close(m);
            errno = ENOMEM;
            return -1;
        }
        buf = tmp;
        cap = new_cap;
    }
    int s = open(buf, O_RDWR | O_NOCTTY);
    if (s < 0) {
        close(m);
        free(buf);
        return -1;
    }
    if (termp)
        tcsetattr(s, TCSAFLUSH, termp);
    if (winp)
        ioctl(s, TIOCSWINSZ, winp);
    if (name)
        strlcpy(name, buf, namesz);
    free(buf);
    *amaster = m;
    *aslave = s;
    return 0;
}

int openpty(int *amaster, int *aslave, char *name, size_t namesz,
            struct termios *termp, struct winsize *winp)
{
    if (!amaster || !aslave)
        return (errno = EINVAL, -1);
    return do_openpty(amaster, aslave, name, namesz, termp, winp);
}

int forkpty(int *amaster, char *name, size_t namesz,
            struct termios *termp, struct winsize *winp)
{
    int m, s;
    if (openpty(&m, &s, name, namesz, termp, winp) < 0)
        return -1;
    pid_t pid = fork();
    if (pid < 0) {
        close(m);
        close(s);
        return -1;
    }
    if (pid == 0) {
        close(m);
        setsid();
#ifdef TIOCSCTTY
        ioctl(s, TIOCSCTTY, 0);
#endif
        dup2(s, STDIN_FILENO);
        dup2(s, STDOUT_FILENO);
        dup2(s, STDERR_FILENO);
        if (s > STDERR_FILENO)
            close(s);
        return 0;
    }
    close(s);
    *amaster = m;
    return pid;
}

#endif

