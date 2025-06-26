/*
 * BSD 2-Clause License
 *
 * Purpose: Implements simple scheduling helpers for vlibc.
 */
#include "sched.h"
#include "time.h"
#include "errno.h"
#include <sys/syscall.h>
#include <unistd.h>
#include "syscall.h"

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
