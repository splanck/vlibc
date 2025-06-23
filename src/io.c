#include "io.h"
#include "errno.h"
#include <sys/types.h>
#include "sys/uio.h"
#include <sys/syscall.h>
#include <unistd.h>
#include "syscall.h"
#include <fcntl.h>
#include <stdarg.h>
#include "sys/stat.h"

#ifndef AT_FDCWD
#define AT_FDCWD -100
#endif


int open(const char *path, int flags, ...)
{
    mode_t mode = 0;
    if (flags & O_CREAT) {
        va_list ap;
        va_start(ap, flags);
        mode = va_arg(ap, mode_t);
        va_end(ap);
    }
#ifdef SYS_open
    long ret = vlibc_syscall(SYS_open, (long)path, flags, mode, 0, 0, 0);
#else
    long ret = vlibc_syscall(SYS_openat, AT_FDCWD, (long)path, flags, mode, 0, 0);
#endif
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
}

ssize_t read(int fd, void *buf, size_t count)
{
    long ret = vlibc_syscall(SYS_read, fd, (long)buf, count, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (ssize_t)ret;
}

ssize_t write(int fd, const void *buf, size_t count)
{
    long ret = vlibc_syscall(SYS_write, fd, (long)buf, count, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (ssize_t)ret;
}

ssize_t readv(int fd, const struct iovec *iov, int iovcnt)
{
#ifdef SYS_readv
    long ret = vlibc_syscall(SYS_readv, fd, (long)iov, iovcnt, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (ssize_t)ret;
#else
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    extern ssize_t host_readv(int, const struct iovec *, int) __asm__("readv");
    return host_readv(fd, iov, iovcnt);
#else
    ssize_t total = 0;
    for (int i = 0; i < iovcnt; i++) {
        const char *base = (const char *)iov[i].iov_base;
        size_t len = iov[i].iov_len;
        size_t off = 0;
        while (off < len) {
            ssize_t r = read(fd, (void *)(base + off), len - off);
            if (r < 0)
                return total ? total : -1;
            if (r == 0)
                return total;
            off += (size_t)r;
            total += r;
        }
    }
    return total;
#endif
#endif
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt)
{
#ifdef SYS_writev
    long ret = vlibc_syscall(SYS_writev, fd, (long)iov, iovcnt, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (ssize_t)ret;
#else
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    extern ssize_t host_writev(int, const struct iovec *, int) __asm__("writev");
    return host_writev(fd, iov, iovcnt);
#else
    ssize_t total = 0;
    for (int i = 0; i < iovcnt; i++) {
        const char *base = (const char *)iov[i].iov_base;
        size_t len = iov[i].iov_len;
        size_t off = 0;
        while (off < len) {
            ssize_t w = write(fd, base + off, len - off);
            if (w < 0)
                return total ? total : -1;
            off += (size_t)w;
            total += w;
            if ((size_t)w < len - off)
                break;
        }
    }
    return total;
#endif
#endif
}

ssize_t pread(int fd, void *buf, size_t count, off_t offset)
{
#ifdef SYS_pread
    long ret = vlibc_syscall(SYS_pread, fd, (long)buf, count, offset, 0, 0);
#elif defined(SYS_pread64)
    long ret = vlibc_syscall(SYS_pread64, fd, (long)buf, count, offset, 0, 0);
#else
    extern ssize_t host_pread(int, void *, size_t, off_t) __asm__("pread");
    return host_pread(fd, buf, count, offset);
#endif
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (ssize_t)ret;
}

ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset)
{
#ifdef SYS_pwrite
    long ret = vlibc_syscall(SYS_pwrite, fd, (long)buf, count, offset, 0, 0);
#elif defined(SYS_pwrite64)
    long ret = vlibc_syscall(SYS_pwrite64, fd, (long)buf, count, offset, 0, 0);
#else
    extern ssize_t host_pwrite(int, const void *, size_t, off_t) __asm__("pwrite");
    return host_pwrite(fd, buf, count, offset);
#endif
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (ssize_t)ret;
}

int close(int fd)
{
    long ret = vlibc_syscall(SYS_close, fd, 0, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
}

int openat(int dirfd, const char *path, int flags, ...)
{
    mode_t mode = 0;
    if (flags & O_CREAT) {
        va_list ap;
        va_start(ap, flags);
        mode = va_arg(ap, mode_t);
        va_end(ap);
    }
#ifdef SYS_openat
    long ret = vlibc_syscall(SYS_openat, dirfd, (long)path, flags, mode, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
#else
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    extern int host_openat(int, const char *, int, ...) __asm__("openat");
    return host_openat(dirfd, path, flags, mode);
#else
    (void)dirfd; (void)path; (void)flags; (void)mode;
    errno = ENOSYS;
    return -1;
#endif
#endif
}

int fstatat(int dirfd, const char *path, struct stat *buf, int flags)
{
#ifdef SYS_fstatat
    long ret = vlibc_syscall(SYS_fstatat, dirfd, (long)path, (long)buf, flags, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
#else
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    extern int host_fstatat(int, const char *, struct stat *, int) __asm__("fstatat");
    return host_fstatat(dirfd, path, buf, flags);
#else
    (void)dirfd; (void)path; (void)buf; (void)flags;
    errno = ENOSYS;
    return -1;
#endif
#endif
}

int unlinkat(int dirfd, const char *pathname, int flags)
{
#ifdef SYS_unlinkat
    long ret = vlibc_syscall(SYS_unlinkat, dirfd, (long)pathname, flags, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
#else
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    extern int host_unlinkat(int, const char *, int) __asm__("unlinkat");
    return host_unlinkat(dirfd, pathname, flags);
#else
    (void)dirfd; (void)pathname; (void)flags;
    errno = ENOSYS;
    return -1;
#endif
#endif
}

int mkdirat(int dirfd, const char *pathname, mode_t mode)
{
#ifdef SYS_mkdirat
    long ret = vlibc_syscall(SYS_mkdirat, dirfd, (long)pathname, mode, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
#else
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    extern int host_mkdirat(int, const char *, mode_t) __asm__("mkdirat");
    return host_mkdirat(dirfd, pathname, mode);
#else
    (void)dirfd; (void)pathname; (void)mode;
    errno = ENOSYS;
    return -1;
#endif
#endif
}

int symlinkat(const char *target, int dirfd, const char *linkpath)
{
#ifdef SYS_symlinkat
    long ret = vlibc_syscall(SYS_symlinkat, (long)target, dirfd, (long)linkpath, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
#else
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    extern int host_symlinkat(const char *, int, const char *) __asm__("symlinkat");
    return host_symlinkat(target, dirfd, linkpath);
#else
    (void)target; (void)dirfd; (void)linkpath;
    errno = ENOSYS;
    return -1;
#endif
#endif
}
