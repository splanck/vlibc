/*
 * BSD 2-Clause License
 *
 * Purpose: Declarations for time related helpers.
 */
#ifndef TIME_H
#define TIME_H

#include <sys/types.h>
#include <stddef.h>
#ifndef __useconds_t_defined
typedef unsigned useconds_t;
#define __useconds_t_defined
#endif

#ifndef _STRUCT_TIMESPEC
#define _STRUCT_TIMESPEC 1
struct timespec {
    time_t tv_sec;
    long tv_nsec;
};
#endif

#ifndef __timeval_defined
#define __timeval_defined 1
struct timeval {
    time_t tv_sec;
    long tv_usec;
};
#endif

struct itimerval {
    struct timeval it_interval; /* timer period */
    struct timeval it_value;    /* time until next expiration */
};

struct itimerspec {
    struct timespec it_interval; /* timer period */
    struct timespec it_value;    /* time until next expiration */
};

#ifndef ITIMER_REAL
#define ITIMER_REAL    0
#endif
#ifndef ITIMER_VIRTUAL
#define ITIMER_VIRTUAL 1
#endif
#ifndef ITIMER_PROF
#define ITIMER_PROF    2
#endif

struct tm {
    int tm_sec;   /* seconds [0,60] */
    int tm_min;   /* minutes [0,59] */
    int tm_hour;  /* hours [0,23] */
    int tm_mday;  /* day of month [1,31] */
    int tm_mon;   /* months since January [0,11] */
    int tm_year;  /* years since 1900 */
    int tm_wday;  /* days since Sunday [0,6] */
    int tm_yday;  /* days since January 1 [0,365] */
int tm_isdst; /* daylight savings time flag */
};

#ifndef __clock_t_defined
#define __clock_t_defined 1
typedef long clock_t;
#endif

#ifndef __clockid_t_defined
#define __clockid_t_defined 1
typedef int clockid_t;
#endif

typedef union sigval {
    int sival_int;
    void *sival_ptr;
} sigval_t;

struct sigevent {
    int sigev_notify;
    int sigev_signo;
    sigval_t sigev_value;
};

#ifndef SIGEV_SIGNAL
#define SIGEV_SIGNAL 0
#define SIGEV_NONE   1
#endif

#ifndef __timer_t_defined
#define __timer_t_defined 1
typedef struct vlibc_timer *timer_t;
#endif

#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME 0
#endif
#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC 1
#endif

int clock_gettime(int clk_id, struct timespec *ts);
/*
 * Retrieve the current value of the given clock. When the
 * SYS_clock_gettime syscall is available it is used; otherwise
 * gettimeofday() provides CLOCK_REALTIME.
 */
int clock_getres(int clk_id, struct timespec *res);
/*
 * Return the resolution for the specified clock ID. On BSD
 * systems the host clock_getres implementation is called if
 * no direct syscall is present.
 */
int clock_settime(int clk_id, const struct timespec *ts);
/*
 * Set the specified clock. When SYS_clock_settime exists it is
 * invoked directly. BSD builds fall back to settimeofday() for
 * CLOCK_REALTIME.
 */


time_t time(time_t *t);
/*
 * Seconds elapsed since the Unix epoch. Uses SYS_time when
 * available, otherwise falls back to clock_gettime or the
 * host time() implementation.
 */
int gettimeofday(struct timeval *tv, void *tz);
/*
 * Populate *tv with CLOCK_REALTIME expressed in seconds and
 * microseconds. Implemented via SYS_time or SYS_clock_gettime
 * with a host fallback when unavailable.
 */

/* sleep helpers */
unsigned sleep(unsigned seconds);
int usleep(useconds_t usec);
int nanosleep(const struct timespec *req, struct timespec *rem);

int setitimer(int which, const struct itimerval *new,
              struct itimerval *old);
int getitimer(int which, struct itimerval *curr);
unsigned int alarm(unsigned int seconds);
/*
 * Schedule a SIGALRM after the given number of seconds. Uses
 * the SYS_alarm syscall or setitimer() when that syscall is
 * not implemented.
 */

#ifndef TIMER_ABSTIME
#define TIMER_ABSTIME 1
#endif

int timer_create(clockid_t clockid, struct sigevent *sevp, timer_t *timerid);
int timer_delete(timer_t timerid);
int timer_settime(timer_t timerid, int flags,
                  const struct itimerspec *new_value,
                  struct itimerspec *old_value);
int timer_gettime(timer_t timerid, struct itimerspec *curr_value);

size_t strftime(char *s, size_t max, const char *format, const struct tm *tm);
char *strptime(const char *s, const char *format, struct tm *tm);

/* basic time conversion helpers */
struct tm *gmtime(const time_t *timep);
/*
 * Convert a time_t to a UTC broken-down time using a static
 * buffer. Not thread-safe.
 */
struct tm *localtime(const time_t *timep);
/*
 * Convert a time_t to local broken-down time using a static
 * buffer. Not thread-safe.
 */
struct tm *gmtime_r(const time_t *timep, struct tm *result);
/*
 * Thread-safe conversion of time_t to UTC broken-down time.
 */
struct tm *localtime_r(const time_t *timep, struct tm *result);
/*
 * Reload timezone information from the TZ environment variable or
 * /etc/localtime and apply the configured offset when converting
 * times.
 */
void tzset(void);
/* Current timezone offset in seconds east of UTC. */
extern int __vlibc_tzoff;
time_t mktime(struct tm *tm);
/* Convert a broken-down UTC time to seconds since the epoch. */
time_t timegm(struct tm *tm);
/* Non-standard alias for mktime(). */
double difftime(time_t end, time_t start);
/* Difference between two times in seconds. */
char *ctime(const time_t *timep);
/* Format a time value using localtime() into a static string. */
char *asctime(const struct tm *tm);
/* Format a broken-down time into a static string. */
char *asctime_r(const struct tm *tm, char *buf);
/* Reentrant version of asctime() writing to user supplied buffer. */

#endif /* TIME_H */
