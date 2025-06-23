#include "signal.h"
#include "errno.h"
#include <sys/syscall.h>
#include "syscall.h"

int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact)
{
#ifdef SYS_rt_sigaction
    long ret = vlibc_syscall(SYS_rt_sigaction, signum, (long)act,
                             (long)oldact, sizeof(sigset_t), 0, 0);
#elif defined(SYS_sigaction)
    long ret = vlibc_syscall(SYS_sigaction, signum, (long)act,
                             (long)oldact, 0, 0, 0);
#else
    long ret = -ENOSYS;
#endif
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
}

int sigprocmask(int how, const sigset_t *set, sigset_t *oldset)
{
#ifdef SYS_rt_sigprocmask
    long ret = vlibc_syscall(SYS_rt_sigprocmask, how, (long)set,
                             (long)oldset, sizeof(sigset_t), 0, 0);
#elif defined(SYS_sigprocmask)
    long ret = vlibc_syscall(SYS_sigprocmask, how, (long)set,
                             (long)oldset, 0, 0, 0);
#else
    long ret = -ENOSYS;
#endif
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
}

int sigemptyset(sigset_t *set)
{
    *set = 0;
    return 0;
}

int sigfillset(sigset_t *set)
{
    *set = ~(sigset_t)0;
    return 0;
}

int sigaddset(sigset_t *set, int signo)
{
    if (signo <= 0 || signo > 8 * (int)sizeof(sigset_t))
        return -1;
    *set |= (sigset_t)1 << (signo - 1);
    return 0;
}

int sigdelset(sigset_t *set, int signo)
{
    if (signo <= 0 || signo > 8 * (int)sizeof(sigset_t))
        return -1;
    *set &= ~((sigset_t)1 << (signo - 1));
    return 0;
}

int sigismember(const sigset_t *set, int signo)
{
    if (signo <= 0 || signo > 8 * (int)sizeof(sigset_t))
        return 0;
    return (*set & ((sigset_t)1 << (signo - 1))) ? 1 : 0;
}
