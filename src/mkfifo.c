/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms, with or without modification, are permitted provided that the copyright notice and this permission notice appear in all copies. This software is provided "as is" without warranty.
 *
 * Purpose: Implements the mkfifo functions for vlibc. Provides wrappers and helpers used by the standard library.
 *
 * Copyright (c) 2025
 */

#include "sys/stat.h"
#include "errno.h"
#include <sys/syscall.h>
#include <unistd.h>
#include "syscall.h"

#ifndef AT_FDCWD
#define AT_FDCWD -100
#endif

#ifndef SYS_mkfifo
extern int host_mkfifo(const char *path, mode_t mode) __asm__("mkfifo");
#endif

#ifndef SYS_mkfifoat
extern int host_mkfifoat(int dirfd, const char *path, mode_t mode) __asm__("mkfifoat");
#endif

int mkfifo(const char *path, mode_t mode)
{
#ifdef SYS_mkfifo
    long ret = vlibc_syscall(SYS_mkfifo, (long)path, mode, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
#else
    return host_mkfifo(path, mode);
#endif
}

int mkfifoat(int dirfd, const char *path, mode_t mode)
{
#ifdef SYS_mkfifoat
    long ret = vlibc_syscall(SYS_mkfifoat, dirfd, (long)path, mode, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
#else
    return host_mkfifoat(dirfd, path, mode);
#endif
}
