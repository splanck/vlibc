/*
 * BSD 2-Clause License
 *
 * Purpose: Minimal futex wrappers for vlibc blocking primitives.
 */
#ifndef VLIBC_FUTEX_H
#define VLIBC_FUTEX_H

#include <time.h>
#include <sys/syscall.h>
#include "syscall.h"
#ifdef __linux__
#include <linux/futex.h>

static inline int futex_wait(atomic_int *addr, int val,
                             const struct timespec *ts)
{
    return (int)vlibc_syscall(SYS_futex, (long)addr,
                              FUTEX_WAIT | FUTEX_PRIVATE_FLAG,
                              val, (long)ts, 0, 0);
}

static inline int futex_wake(atomic_int *addr, int count)
{
    return (int)vlibc_syscall(SYS_futex, (long)addr,
                              FUTEX_WAKE | FUTEX_PRIVATE_FLAG,
                              count, 0, 0, 0);
}
#else
static inline int futex_wait(atomic_int *addr, int val,
                             const struct timespec *ts)
{
    (void)addr; (void)val; nanosleep(ts, NULL); return 0;
}
static inline int futex_wake(atomic_int *addr, int count)
{
    (void)addr; (void)count; return 0;
}
#endif

#endif /* VLIBC_FUTEX_H */
