/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms, with or without modification, are permitted provided that the copyright notice and this permission notice appear in all copies. This software is provided "as is" without warranty.
 *
 * Purpose: Implements POSIX shared memory wrappers for vlibc.
 */

#include "sys/shm.h"
#include "errno.h"
#include <sys/syscall.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "syscall.h"

int shm_open(const char *name, int oflag, mode_t mode)
{
#ifdef SYS_shm_open
    long ret = vlibc_syscall(SYS_shm_open, (long)name, oflag, mode, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
#elif defined(__FreeBSD__) || defined(__NetBSD__) || \
      defined(__OpenBSD__) || defined(__DragonFly__)
    extern int host_shm_open(const char *, int, mode_t) __asm__("shm_open");
    return host_shm_open(name, oflag, mode);
#else
    if (!name || name[0] != '/') {
        errno = EINVAL;
        return -1;
    }
    char path[256];
    snprintf(path, sizeof(path), "/dev/shm%s", name);
    return open(path, oflag, mode);
#endif
}

int shm_unlink(const char *name)
{
#ifdef SYS_shm_unlink
    long ret = vlibc_syscall(SYS_shm_unlink, (long)name, 0, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
#elif defined(__FreeBSD__) || defined(__NetBSD__) || \
      defined(__OpenBSD__) || defined(__DragonFly__)
    extern int host_shm_unlink(const char *) __asm__("shm_unlink");
    return host_shm_unlink(name);
#else
    if (!name || name[0] != '/') {
        errno = EINVAL;
        return -1;
    }
    char path[256];
    snprintf(path, sizeof(path), "/dev/shm%s", name);
    return unlink(path);
#endif
}
