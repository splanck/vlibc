/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms, with or without modification, are permitted provided that the copyright notice and this permission notice appear in all copies. This software is provided "as is" without warranty.
 *
 * Purpose: Implements the getlogin functions for vlibc. Provides wrappers and helpers used by the standard library.
 *
 * Copyright (c) 2025
 */

#include "unistd.h"
#include "pwd.h"
#include "string.h"
#include "errno.h"

/*
 * Obtain the login name for the current user by calling
 * getpwuid(getuid()). The resulting pw_name field is copied into a
 * thread-local buffer so repeated calls avoid further lookups.
 */
static int getlogin_impl(char *buf, size_t len)
{
    if (!buf || len == 0)
        return EINVAL;

    struct passwd *pw = getpwuid(getuid());
    if (!pw || !pw->pw_name)
        return ENOENT;

    size_t n = strlcpy(buf, pw->pw_name, len);
    if (n >= len)
        return ERANGE;
    return 0;
}

int getlogin_r(char *buf, size_t len)
{
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    return getlogin_impl(buf, len);
#else
    extern int host_getlogin_r(char *, size_t) __asm("getlogin_r");
    return host_getlogin_r(buf, len);
#endif
}

char *getlogin(void)
{
    static __thread char name[64];
    if (name[0])
        return name;

    if (getlogin_r(name, sizeof(name)) != 0)
        return NULL;
    return name;
}
