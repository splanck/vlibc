#include "sys/stat.h"
#include "errno.h"
#include <sys/syscall.h>
#include <unistd.h>
#include "syscall.h"

int stat(const char *path, struct stat *buf)
{
    long ret = vlibc_syscall(SYS_stat, (long)path, (long)buf, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
}

int fstat(int fd, struct stat *buf)
{
    long ret = vlibc_syscall(SYS_fstat, fd, (long)buf, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
}

int lstat(const char *path, struct stat *buf)
{
    long ret = vlibc_syscall(SYS_lstat, (long)path, (long)buf, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
}
