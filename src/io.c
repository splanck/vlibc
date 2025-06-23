#include "io.h"
#include "errno.h"
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include "syscall.h"
#include <fcntl.h>
#include <stdarg.h>


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
