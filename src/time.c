#include "time.h"
#include "errno.h"
#include <sys/syscall.h>
#include <unistd.h>
#include "syscall.h"

#ifdef SYS_clock_gettime
#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME 0
#endif
#endif

time_t time(time_t *t)
{
#ifdef SYS_time
    long ret = vlibc_syscall(SYS_time, (long)t, 0, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return (time_t)-1;
    }
    time_t sec = (time_t)ret;
    if (t)
        *t = sec;
    return sec;
#elif defined(SYS_clock_gettime)
    struct timespec ts;
    long ret = vlibc_syscall(SYS_clock_gettime, CLOCK_REALTIME, (long)&ts, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return (time_t)-1;
    }
    if (t)
        *t = ts.tv_sec;
    return ts.tv_sec;
#else
    (void)t;
    return (time_t)-1;
#endif
}

int gettimeofday(struct timeval *tv, void *tz)
{
    (void)tz;
#ifdef SYS_time
    long ret = vlibc_syscall(SYS_time, 0, 0, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    time_t sec = (time_t)ret;
    if (tv) {
        tv->tv_sec = sec;
        tv->tv_usec = 0;
    }
    return 0;
#elif defined(SYS_clock_gettime)
    struct timespec ts;
    long ret = vlibc_syscall(SYS_clock_gettime, CLOCK_REALTIME, (long)&ts, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    if (tv) {
        tv->tv_sec = ts.tv_sec;
        tv->tv_usec = ts.tv_nsec / 1000;
    }
    return 0;
#else
    (void)tv;
    return -1;
#endif
}
