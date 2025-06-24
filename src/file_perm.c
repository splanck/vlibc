/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms, with or without modification, are permitted provided that the copyright notice and this permission notice appear in all copies. This software is provided "as is" without warranty.
 *
 * Purpose: Implements the file_perm functions for vlibc. Provides wrappers and helpers used by the standard library.
 *
 * Copyright (c) 2025
 */

#include "sys/file.h"
#include "errno.h"
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include "syscall.h"

int chmod(const char *path, mode_t mode)
{
    long ret = vlibc_syscall(SYS_chmod, (long)path, mode, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
}

int chown(const char *path, uid_t owner, gid_t group)
{
    long ret = vlibc_syscall(SYS_chown, (long)path, owner, group, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
}

mode_t umask(mode_t mask)
{
    long ret = vlibc_syscall(SYS_umask, mask, 0, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return (mode_t)-1;
    }
    return (mode_t)ret;
}

int fchmod(int fd, mode_t mode)
{
#ifdef SYS_fchmod
    long ret = vlibc_syscall(SYS_fchmod, fd, mode, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
#else
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    extern int host_fchmod(int, mode_t) __asm__("fchmod");
    return host_fchmod(fd, mode);
#else
    (void)fd; (void)mode; errno = ENOSYS; return -1;
#endif
#endif
}

int fchmodat(int dirfd, const char *pathname, mode_t mode, int flags)
{
#ifdef SYS_fchmodat
    long ret = vlibc_syscall(SYS_fchmodat, dirfd, (long)pathname, mode, flags, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
#else
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    extern int host_fchmodat(int, const char *, mode_t, int) __asm__("fchmodat");
    return host_fchmodat(dirfd, pathname, mode, flags);
#else
    (void)dirfd; (void)pathname; (void)mode; (void)flags; errno = ENOSYS; return -1;
#endif
#endif
}

int fchown(int fd, uid_t owner, gid_t group)
{
#ifdef SYS_fchown
    long ret = vlibc_syscall(SYS_fchown, fd, owner, group, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
#else
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    extern int host_fchown(int, uid_t, gid_t) __asm__("fchown");
    return host_fchown(fd, owner, group);
#else
    (void)fd; (void)owner; (void)group; errno = ENOSYS; return -1;
#endif
#endif
}

int fchownat(int dirfd, const char *pathname, uid_t owner, gid_t group, int flags)
{
#ifdef SYS_fchownat
    long ret = vlibc_syscall(SYS_fchownat, dirfd, (long)pathname, owner, group, flags, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
#else
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    extern int host_fchownat(int, const char *, uid_t, gid_t, int) __asm__("fchownat");
    return host_fchownat(dirfd, pathname, owner, group, flags);
#else
    (void)dirfd; (void)pathname; (void)owner; (void)group; (void)flags; errno = ENOSYS; return -1;
#endif
#endif
}

int lchown(const char *pathname, uid_t owner, gid_t group)
{
#ifdef SYS_lchown
    long ret = vlibc_syscall(SYS_lchown, (long)pathname, owner, group, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
#else
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    extern int host_lchown(const char *, uid_t, gid_t) __asm__("lchown");
    return host_lchown(pathname, owner, group);
#else
    (void)pathname; (void)owner; (void)group; errno = ENOSYS; return -1;
#endif
#endif
}
