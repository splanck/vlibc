/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms, with or without modification, are permitted provided that the copyright notice and this permission notice appear in all copies. This software is provided "as is" without warranty.
 *
 * Purpose: Implements the pathconf functions for vlibc. Provides wrappers and helpers used by the standard library.
 *
 * Copyright (c) 2025
 */

#include "unistd.h"
#include "sys/statvfs.h"
#include "errno.h"
#include <limits.h>

long pathconf(const char *path, int name)
{
    struct statvfs sv;
    switch (name) {
    case _PC_NAME_MAX:
        if (statvfs(path, &sv) < 0)
            return -1;
        return (long)sv.f_namemax;
    case _PC_PATH_MAX:
#ifdef PATH_MAX
        return PATH_MAX;
#else
        return 4096;
#endif
    default:
        errno = EINVAL;
        return -1;
    }
}

long fpathconf(int fd, int name)
{
    struct statvfs sv;
    switch (name) {
    case _PC_NAME_MAX:
        if (fstatvfs(fd, &sv) < 0)
            return -1;
        return (long)sv.f_namemax;
    case _PC_PATH_MAX:
#ifdef PATH_MAX
        return PATH_MAX;
#else
        return 4096;
#endif
    default:
        errno = EINVAL;
        return -1;
    }
}
