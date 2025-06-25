/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms, with or without modification, are permitted provided that the copyright notice and this permission notice appear in all copies. This software is provided "as is" without warranty.
 *
 * Purpose: Implements the signal functions for vlibc. Provides wrappers and helpers used by the standard library.
 *
 * Copyright (c) 2025
 */

#include "signal.h"
#include "errno.h"
#include <sys/syscall.h>
#include "syscall.h"
#include <stddef.h>

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

struct sig_name {
    int num;
    const char *name;
};

static const struct sig_name sig_names[] = {
    { SIGHUP, "Hangup" },
    { SIGINT, "Interrupt" },
    { SIGQUIT, "Quit" },
    { SIGILL, "Illegal instruction" },
    { SIGTRAP, "Trace/breakpoint trap" },
    { SIGABRT, "Aborted" },
    { SIGBUS, "Bus error" },
    { SIGFPE, "Floating point exception" },
    { SIGKILL, "Killed" },
    { SIGUSR1, "User signal 1" },
    { SIGSEGV, "Segmentation fault" },
    { SIGUSR2, "User signal 2" },
    { SIGPIPE, "Broken pipe" },
    { SIGALRM, "Alarm clock" },
    { SIGTERM, "Terminated" },
    { SIGCHLD, "Child exited" },
    { SIGCONT, "Continued" },
    { SIGSTOP, "Stopped (signal)" },
    { SIGTSTP, "Stopped" },
    { SIGTTIN, "Stopped (tty input)" },
    { SIGTTOU, "Stopped (tty output)" },
    { 0, NULL }
};

char *strsignal(int signum)
{
    for (size_t i = 0; sig_names[i].name; ++i) {
        if (sig_names[i].num == signum)
            return (char *)sig_names[i].name;
    }
    return "Unknown signal";
}

/* Retrieve the set of pending signals */
int sigpending(sigset_t *set)
{
#ifdef SYS_rt_sigpending
    long ret = vlibc_syscall(SYS_rt_sigpending, (long)set,
                             sizeof(sigset_t), 0, 0, 0, 0);
#elif defined(SYS_sigpending)
    long ret = vlibc_syscall(SYS_sigpending, (long)set, 0, 0, 0, 0, 0);
#else
    long ret = -ENOSYS;
#endif
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
}

/* Suspend execution until a signal arrives */
int sigsuspend(const sigset_t *mask)
{
#ifdef SYS_rt_sigsuspend
    long ret = vlibc_syscall(SYS_rt_sigsuspend, (long)mask,
                             sizeof(sigset_t), 0, 0, 0, 0);
#elif defined(SYS_sigsuspend)
    long ret = vlibc_syscall(SYS_sigsuspend, (long)mask, 0, 0, 0, 0, 0);
#else
    long ret = -ENOSYS;
#endif
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
}

/* Pause the process until a signal is caught */
int pause(void)
{
#ifdef SYS_pause
    long ret = vlibc_syscall(SYS_pause, 0, 0, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
#else
    sigset_t mask;
    sigemptyset(&mask);
    return sigsuspend(&mask);
#endif
}

/* Wait for a signal from the set with optional timeout */
int sigtimedwait(const sigset_t *set, siginfo_t *info,
                 const struct timespec *timeout)
{
#ifdef SYS_rt_sigtimedwait_time64
    long ret = vlibc_syscall(SYS_rt_sigtimedwait_time64, (long)set,
                             (long)info, (long)timeout,
                             sizeof(sigset_t), 0, 0);
#elif defined(SYS_rt_sigtimedwait)
    long ret = vlibc_syscall(SYS_rt_sigtimedwait, (long)set,
                             (long)info, (long)timeout,
                             sizeof(sigset_t), 0, 0);
#else
    (void)set; (void)info; (void)timeout;
    long ret = -ENOSYS;
#endif
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
}

/* Wait for a signal from the set */
int sigwaitinfo(const sigset_t *set, siginfo_t *info)
{
    return sigtimedwait(set, info, NULL);
}

/* Wait for a signal and return it */
int sigwait(const sigset_t *set, int *sig)
{
    siginfo_t info;
    int r = sigtimedwait(set, &info, NULL);
    if (r < 0)
        return errno;
    if (sig)
        *sig = info.si_signo;
    return 0;
}
