#include "io.h"
#include "errno.h"
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include "syscall.h"
#include "vlibc_features.h"

off_t lseek(int fd, off_t offset, int whence)
{
    long ret = vlibc_syscall(SYS_lseek, fd, (long)offset, whence, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return (off_t)-1;
    }
    return (off_t)ret;
}

int dup(int oldfd)
{
    long ret = vlibc_syscall(SYS_dup, oldfd, 0, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
}

int dup2(int oldfd, int newfd)
{
#if VLIBC_HAVE_DUP3
    long ret = vlibc_syscall(SYS_dup3, oldfd, newfd, 0, 0, 0, 0);
#else
    long ret = vlibc_syscall(SYS_dup2, oldfd, newfd, 0, 0, 0, 0);
#endif
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
}

int pipe(int pipefd[2])
{
#if VLIBC_HAVE_PIPE2
    long ret = vlibc_syscall(SYS_pipe2, (long)pipefd, 0, 0, 0, 0, 0);
#else
    long ret = vlibc_syscall(SYS_pipe, (long)pipefd, 0, 0, 0, 0, 0);
#endif
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
}

int dup3(int oldfd, int newfd, int flags)
{
#if VLIBC_HAVE_DUP3
    long ret = vlibc_syscall(SYS_dup3, oldfd, newfd, flags, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
#else
    if (flags != 0) {
        errno = EINVAL;
        return -1;
    }
    return dup2(oldfd, newfd);
#endif
}

int pipe2(int pipefd[2], int flags)
{
#if VLIBC_HAVE_PIPE2
    long ret = vlibc_syscall(SYS_pipe2, (long)pipefd, flags, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
#else
    if (flags != 0) {
        errno = EINVAL;
        return -1;
    }
    return pipe(pipefd);
#endif
}
