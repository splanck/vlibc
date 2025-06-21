#include "io.h"
#include "errno.h"
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>

extern long syscall(long number, ...);

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
    long ret = syscall(SYS_open, path, flags, mode);
#else
    long ret = syscall(SYS_openat, AT_FDCWD, path, flags, mode);
#endif
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
}

ssize_t read(int fd, void *buf, size_t count)
{
    long ret = syscall(SYS_read, fd, buf, count);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (ssize_t)ret;
}

ssize_t write(int fd, const void *buf, size_t count)
{
    long ret = syscall(SYS_write, fd, buf, count);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (ssize_t)ret;
}

int close(int fd)
{
    long ret = syscall(SYS_close, fd);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
}
