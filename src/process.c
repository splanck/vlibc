#include "process.h"
#include "errno.h"
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

extern long syscall(long number, ...);

pid_t fork(void)
{
#ifdef SYS_fork
    long ret = syscall(SYS_fork);
#else
    long ret = syscall(SYS_clone, SIGCHLD, 0);
#endif
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (pid_t)ret;
}

int execve(const char *pathname, char *const argv[], char *const envp[])
{
    long ret = syscall(SYS_execve, pathname, argv, envp);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
}

pid_t waitpid(pid_t pid, int *status, int options)
{
#ifdef SYS_waitpid
    long ret = syscall(SYS_waitpid, pid, status, options);
#else
    long ret = syscall(SYS_wait4, pid, status, options, NULL);
#endif
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (pid_t)ret;
}

int kill(pid_t pid, int sig)
{
    long ret = syscall(SYS_kill, pid, sig);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
}


void _exit(int status)
{
    syscall(SYS_exit, status);
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
    memset(&act, 0, sizeof(act));
    act.sa_handler = handler;
    long ret = syscall(SYS_rt_sigaction, signum, &act, &old, sizeof(sigset_t));
    if (ret < 0) {
        errno = -ret;
        return SIG_ERR;
    }
    return old.sa_handler;
#else
    long ret = syscall(SYS_signal, signum, handler);
    if (ret < 0) {
        errno = -ret;
        return SIG_ERR;
    }
    return (sighandler_t)ret;
#endif
}
