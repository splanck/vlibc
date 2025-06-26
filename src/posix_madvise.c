/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms,
 * with or without modification, are permitted provided that the copyright
 * notice and this permission notice appear in all copies. This software is
 * provided "as is" without warranty.
 *
 * Purpose: Implement posix_madvise for vlibc. The function wraps madvise
 * and returns POSIX error codes instead of setting errno.
 */

#include "sys/mman.h"
#include "errno.h"

int posix_madvise(void *addr, size_t len, int advice)
{
    int r = madvise(addr, len, advice);
    return r == -1 ? errno : 0;
}
