#include "process.h"
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

extern long syscall(long number, ...);

pid_t fork(void)
{
#ifdef SYS_fork
    return (pid_t)syscall(SYS_fork);
#else
    return (pid_t)syscall(SYS_clone, SIGCHLD, 0);
#endif
}

int execve(const char *pathname, char *const argv[], char *const envp[])
{
    return (int)syscall(SYS_execve, pathname, argv, envp);
}

pid_t waitpid(pid_t pid, int *status, int options)
{
#ifdef SYS_waitpid
    return (pid_t)syscall(SYS_waitpid, pid, status, options);
#else
    return (pid_t)syscall(SYS_wait4, pid, status, options, NULL);
#endif
}

int kill(pid_t pid, int sig)
{
    return (int)syscall(SYS_kill, pid, sig);
}

sighandler_t signal(int signum, sighandler_t handler)
{
#ifdef SYS_rt_sigaction
    struct sigaction act, old;
    memset(&act, 0, sizeof(act));
    act.sa_handler = handler;
    if (syscall(SYS_rt_sigaction, signum, &act, &old, sizeof(sigset_t)) < 0)
        return SIG_ERR;
    return old.sa_handler;
#else
    return (sighandler_t)syscall(SYS_signal, signum, handler);
#endif
}
