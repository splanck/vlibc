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
