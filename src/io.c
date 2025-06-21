#include "io.h"
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
    return (int)syscall(SYS_open, path, flags, mode);
#else
    return (int)syscall(SYS_openat, AT_FDCWD, path, flags, mode);
#endif
}

ssize_t read(int fd, void *buf, size_t count)
{
    return (ssize_t)syscall(SYS_read, fd, buf, count);
}

ssize_t write(int fd, const void *buf, size_t count)
{
    return (ssize_t)syscall(SYS_write, fd, buf, count);
}

int close(int fd)
{
    return (int)syscall(SYS_close, fd);
}
