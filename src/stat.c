#include "sys/stat.h"
#include <sys/syscall.h>
#include <unistd.h>

extern long syscall(long number, ...);

int stat(const char *path, struct stat *buf)
{
    return (int)syscall(SYS_stat, path, buf);
}

int fstat(int fd, struct stat *buf)
{
    return (int)syscall(SYS_fstat, fd, buf);
}

int lstat(const char *path, struct stat *buf)
{
    return (int)syscall(SYS_lstat, path, buf);
}
