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
