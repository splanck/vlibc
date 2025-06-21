#include "process.h"
#include "errno.h"
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <signal.h>
#include "string.h"
#include "syscall.h"
extern long syscall(long number, ...);


pid_t fork(void)
{
#ifdef SYS_fork
    long ret = vlibc_syscall(SYS_fork, 0, 0, 0, 0, 0, 0);
#else
    long ret = vlibc_syscall(SYS_clone, SIGCHLD, 0, 0, 0, 0, 0);
#endif
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (pid_t)ret;
}

int execve(const char *pathname, char *const argv[], char *const envp[])
{
    long ret = vlibc_syscall(SYS_execve, (long)pathname, (long)argv, (long)envp, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
}

pid_t waitpid(pid_t pid, int *status, int options)
{
#ifdef SYS_waitpid
    long ret = vlibc_syscall(SYS_waitpid, pid, (long)status, options, 0, 0, 0);
#else
    long ret = vlibc_syscall(SYS_wait4, pid, (long)status, options, 0, 0, 0);
#endif
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (pid_t)ret;
}

int kill(pid_t pid, int sig)
{
    long ret = vlibc_syscall(SYS_kill, pid, sig, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
}


void _exit(int status)
{
    vlibc_syscall(SYS_exit, status, 0, 0, 0, 0, 0);
    for (;;) {}
}

void exit(int status)
{
    /* No stdio buffering yet, so nothing to flush. */
    _exit(status);
}

sighandler_t signal(int signum, sighandler_t handler)
{
#ifdef SYS_rt_sigaction
    struct sigaction act, old;
    vmemset(&act, 0, sizeof(act));
    act.sa_handler = handler;
    long ret = vlibc_syscall(SYS_rt_sigaction, signum, (long)&act, (long)&old, sizeof(sigset_t), 0, 0);
    if (ret < 0) {
        errno = -ret;
        return SIG_ERR;
    }
    return old.sa_handler;
#else
    long ret = vlibc_syscall(SYS_signal, signum, (long)handler, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return SIG_ERR;
    }
    return (sighandler_t)ret;
#endif
}
