/*
 * BSD 2-Clause "Simplified" License
 *
 * Copyright (c) 2025
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Purpose: Implements simple scheduling helpers for vlibc.
 */
#include "sched.h"
#include "time.h"
#include "errno.h"
#include <sys/syscall.h>
#include <unistd.h>
#include "syscall.h"

/* Yield the processor to another runnable task. */
int sched_yield(void)
{
#ifdef SYS_sched_yield
    long ret = vlibc_syscall(SYS_sched_yield, 0, 0, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
#else
    struct timespec ts = {0, 0};
    return nanosleep(&ts, NULL);
#endif
}

/*
 * Retrieve the current scheduling priority for the given target.
 */
int getpriority(int which, int who)
{
#ifdef SYS_getpriority
    long ret = vlibc_syscall(SYS_getpriority, which, who, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
#elif defined(__FreeBSD__) || defined(__NetBSD__) || \
      defined(__OpenBSD__) || defined(__DragonFly__)
    extern int host_getpriority(int, int) __asm("getpriority");
    return host_getpriority(which, who);
#else
    (void)which; (void)who;
    errno = ENOSYS;
    return -1;
#endif
}

/*
 * Set the scheduling priority for the specified process, process
 * group or user.
 */
int setpriority(int which, int who, int prio)
{
#ifdef SYS_setpriority
    long ret = vlibc_syscall(SYS_setpriority, which, who, prio, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
#elif defined(__FreeBSD__) || defined(__NetBSD__) || \
      defined(__OpenBSD__) || defined(__DragonFly__)
    extern int host_setpriority(int, int, int) __asm("setpriority");
    return host_setpriority(which, who, prio);
#else
    (void)which; (void)who; (void)prio;
    errno = ENOSYS;
    return -1;
#endif
}

/*
 * Increase the calling process' nice value by incr and return the new value.
 */
int nice(int incr)
{
    int cur = getpriority(PRIO_PROCESS, 0);
    if (cur == -1 && errno)
        return -1;
    if (setpriority(PRIO_PROCESS, 0, cur + incr) < 0)
        return -1;
    return cur + incr;
}

/* Return the current scheduling policy for pid. */
int sched_getscheduler(pid_t pid)
{
#ifdef SYS_sched_getscheduler
    long ret = vlibc_syscall(SYS_sched_getscheduler, pid, 0, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
#elif defined(__FreeBSD__) || defined(__NetBSD__) || \
      defined(__OpenBSD__) || defined(__DragonFly__)
    extern int host_sched_getscheduler(pid_t) __asm("sched_getscheduler");
    return host_sched_getscheduler(pid);
#else
    (void)pid;
    errno = ENOSYS;
    return -1;
#endif
}

/* Set the scheduling policy and parameters for pid. */
int sched_setscheduler(pid_t pid, int policy, const struct sched_param *param)
{
#ifdef SYS_sched_setscheduler
    long ret = vlibc_syscall(SYS_sched_setscheduler, pid, policy,
                             (long)param, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
#elif defined(__FreeBSD__) || defined(__NetBSD__) || \
      defined(__OpenBSD__) || defined(__DragonFly__)
    extern int host_sched_setscheduler(pid_t, int, const struct sched_param *)
        __asm("sched_setscheduler");
    return host_sched_setscheduler(pid, policy, param);
#else
    (void)pid; (void)policy; (void)param;
    errno = ENOSYS;
    return -1;
#endif
}

/* Get the scheduling parameters for pid. */
int sched_getparam(pid_t pid, struct sched_param *param)
{
#ifdef SYS_sched_getparam
    long ret = vlibc_syscall(SYS_sched_getparam, pid, (long)param,
                             0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
#elif defined(__FreeBSD__) || defined(__NetBSD__) || \
      defined(__OpenBSD__) || defined(__DragonFly__)
    extern int host_sched_getparam(pid_t, struct sched_param *)
        __asm("sched_getparam");
    return host_sched_getparam(pid, param);
#else
    (void)pid; (void)param;
    errno = ENOSYS;
    return -1;
#endif
}

/* Set the scheduling parameters for pid. */
int sched_setparam(pid_t pid, const struct sched_param *param)
{
#ifdef SYS_sched_setparam
    long ret = vlibc_syscall(SYS_sched_setparam, pid, (long)param,
                             0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
#elif defined(__FreeBSD__) || defined(__NetBSD__) || \
      defined(__OpenBSD__) || defined(__DragonFly__)
    extern int host_sched_setparam(pid_t, const struct sched_param *)
        __asm("sched_setparam");
    return host_sched_setparam(pid, param);
#else
    (void)pid; (void)param;
    errno = ENOSYS;
    return -1;
#endif
}

/* Return the maximum scheduling priority for the given policy. */
int sched_get_priority_max(int policy)
{
#ifdef SYS_sched_get_priority_max
    long ret = vlibc_syscall(SYS_sched_get_priority_max, policy,
                             0, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
#elif defined(__FreeBSD__) || defined(__NetBSD__) || \
      defined(__OpenBSD__) || defined(__DragonFly__)
    extern int host_sched_get_priority_max(int) __asm("sched_get_priority_max");
    return host_sched_get_priority_max(policy);
#else
    (void)policy;
    errno = ENOSYS;
    return -1;
#endif
}

/* Return the minimum scheduling priority for the given policy. */
int sched_get_priority_min(int policy)
{
#ifdef SYS_sched_get_priority_min
    long ret = vlibc_syscall(SYS_sched_get_priority_min, policy,
                             0, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
#elif defined(__FreeBSD__) || defined(__NetBSD__) || \
      defined(__OpenBSD__) || defined(__DragonFly__)
    extern int host_sched_get_priority_min(int) __asm("sched_get_priority_min");
    return host_sched_get_priority_min(policy);
#else
    (void)policy;
    errno = ENOSYS;
    return -1;
#endif
}

/* Obtain the round-robin time quantum for pid. */
int sched_rr_get_interval(pid_t pid, struct timespec *interval)
{
#ifdef SYS_sched_rr_get_interval_time64
    long ret = vlibc_syscall(SYS_sched_rr_get_interval_time64, pid,
                             (long)interval, 0, 0, 0, 0);
#elif defined(SYS_sched_rr_get_interval)
    long ret = vlibc_syscall(SYS_sched_rr_get_interval, pid,
                             (long)interval, 0, 0, 0, 0);
#else
    (void)pid; (void)interval;
    long ret = -ENOSYS;
#endif
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
}
