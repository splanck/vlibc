/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms, with or without modification, are permitted provided that the copyright notice and this permission notice appear in all copies. This software is provided "as is" without warranty.
 *
 * Purpose: Implements the syscall functions for vlibc. Provides wrappers and helpers used by the standard library.
 *
 * Copyright (c) 2025
 */

#include <stdarg.h>
#include_next <errno.h>
#include "syscall.h"

long vlibc_syscall(long number, ...)
{
    (void)number;
    errno = ENOSYS;
    return -1;
}
