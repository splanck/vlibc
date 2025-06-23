#include "time.h"
#include "errno.h"
#include <sys/syscall.h>
#include <unistd.h>
#include "syscall.h"
#if !defined(SYS_time) && !defined(SYS_clock_gettime)
#include_next <time.h>
#include <sys/time.h>
extern time_t host_time(time_t *t) __asm__("time");
extern int host_gettimeofday(struct timeval *tv, void *tz)
    __asm__("gettimeofday");
#endif

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
    return host_time(t);
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
    return host_gettimeofday(tv, tz);
#endif
}

unsigned int alarm(unsigned int seconds)
{
#ifdef SYS_alarm
    long ret = vlibc_syscall(SYS_alarm, seconds, 0, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return 0;
    }
    return (unsigned int)ret;
#else
    struct itimerval it = { {0, 0}, { (time_t)seconds, 0 } };
    struct itimerval old;
    if (setitimer(ITIMER_REAL, &it, &old) < 0)
        return 0;
    return (unsigned int)old.it_value.tv_sec;
#endif
}
