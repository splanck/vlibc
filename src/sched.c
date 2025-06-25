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
