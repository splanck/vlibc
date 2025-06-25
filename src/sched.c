/*
 * BSD 2-Clause License
 *
 * Purpose: Implements simple scheduling helpers for vlibc.
 */
#include "sched.h"
#include "errno.h"
#include <sys/syscall.h>
#include <unistd.h>
#include "syscall.h"

int sched_yield(void)
{
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__) || defined(__APPLE__)
# ifdef SYS_sched_yield
    long ret = syscall(SYS_sched_yield, 0, 0, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
# else
    extern int host_sched_yield(void) __asm__("sched_yield");
    return host_sched_yield();
# endif
#else
# ifdef SYS_sched_yield
    long ret = vlibc_syscall(SYS_sched_yield, 0, 0, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
# else
    extern int host_sched_yield(void) __asm__("sched_yield");
    return host_sched_yield();
# endif
#endif
}
