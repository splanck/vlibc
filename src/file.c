#include "io.h"
#include "errno.h"
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <fcntl.h>
#ifndef AT_FDCWD
#define AT_FDCWD -100
#endif
#include "syscall.h"

int unlink(const char *pathname)
{
    long ret = vlibc_syscall(SYS_unlinkat, AT_FDCWD, (long)pathname, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
}

int rename(const char *oldpath, const char *newpath)
{
    long ret = vlibc_syscall(SYS_renameat, AT_FDCWD, (long)oldpath, AT_FDCWD,
                             (long)newpath, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
}

int link(const char *oldpath, const char *newpath)
{
    long ret = vlibc_syscall(SYS_linkat, AT_FDCWD, (long)oldpath, AT_FDCWD,
                             (long)newpath, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
}

int symlink(const char *target, const char *linkpath)
{
    long ret = vlibc_syscall(SYS_symlinkat, (long)target, AT_FDCWD,
                             (long)linkpath, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
}

ssize_t readlink(const char *pathname, char *buf, size_t bufsiz)
{
    long ret = vlibc_syscall(SYS_readlinkat, AT_FDCWD, (long)pathname,
                             (long)buf, bufsiz, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (ssize_t)ret;
}

int chdir(const char *path)
{
    long ret = vlibc_syscall(SYS_chdir, (long)path, 0, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
}
