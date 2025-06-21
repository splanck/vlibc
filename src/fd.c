#include "io.h"
#include "errno.h"
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include "syscall.h"

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
#ifdef SYS_dup3
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
#ifdef SYS_pipe2
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
