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

int close(int fd)
{
    long ret = vlibc_syscall(SYS_close, fd, 0, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
}
