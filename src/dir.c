/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms, with or without modification, are permitted provided that the copyright notice and this permission notice appear in all copies. This software is provided "as is" without warranty.
 *
 * Purpose: Implements the dir functions for vlibc. Provides wrappers and helpers used by the standard library.
 *
 * Copyright (c) 2025
 */

#include "io.h"
#include "errno.h"
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include "syscall.h"

#ifndef AT_FDCWD
#define AT_FDCWD -100
#endif

int mkdir(const char *pathname, mode_t mode)
{
#ifdef SYS_mkdir
    long ret = vlibc_syscall(SYS_mkdir, (long)pathname, mode, 0, 0, 0, 0);
#else
    long ret = vlibc_syscall(SYS_mkdirat, AT_FDCWD, (long)pathname, mode, 0, 0, 0);
#endif
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
}

int rmdir(const char *pathname)
{
    long ret = vlibc_syscall(SYS_rmdir, (long)pathname, 0, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
}

