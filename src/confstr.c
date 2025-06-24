/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms, with or without modification, are permitted provided that the copyright notice and this permission notice appear in all copies. This software is provided "as is" without warranty.
 *
 * Purpose: Implements the confstr functions for vlibc. Provides wrappers and helpers used by the standard library.
 *
 * Copyright (c) 2025
 */

#include "unistd.h"
#include "errno.h"

#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
extern size_t host_confstr(int, char *, size_t) __asm("confstr");

size_t confstr(int name, char *buf, size_t len)
{
    return host_confstr(name, buf, len);
}
#else
size_t confstr(int name, char *buf, size_t len)
{
    (void)name;
    (void)buf;
    (void)len;
    errno = EINVAL;
    return 0;
}
#endif
