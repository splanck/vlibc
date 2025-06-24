/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms, with or without modification, are permitted provided that the copyright notice and this permission notice appear in all copies. This software is provided "as is" without warranty.
 *
 * Purpose: Implements the syscall functions for vlibc. Provides wrappers and helpers used by the standard library.
 *
 * Copyright (c) 2025
 */

#define _GNU_SOURCE
#include <stdarg.h>
#include <unistd.h>
#include <sys/syscall.h>
#include_next <errno.h>

long vlibc_syscall(long number, ...)
{
    va_list ap;
    va_start(ap, number);
    unsigned long a1 = va_arg(ap, unsigned long);
    unsigned long a2 = va_arg(ap, unsigned long);
    unsigned long a3 = va_arg(ap, unsigned long);
    unsigned long a4 = va_arg(ap, unsigned long);
    unsigned long a5 = va_arg(ap, unsigned long);
    unsigned long a6 = va_arg(ap, unsigned long);
    va_end(ap);

    long ret = syscall(number, a1, a2, a3, a4, a5, a6);
    if (ret < 0)
        return -errno;
    return ret;
}
