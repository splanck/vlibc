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

#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME 0
#endif
#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC 1
#endif

int clock_gettime(int clk_id, struct timespec *ts);

time_t time(time_t *t);
int gettimeofday(struct timeval *tv, void *tz);

/* sleep helpers */
unsigned sleep(unsigned seconds);
int usleep(useconds_t usec);
int nanosleep(const struct timespec *req, struct timespec *rem);

int setitimer(int which, const struct itimerval *new,
              struct itimerval *old);
int getitimer(int which, struct itimerval *curr);

size_t strftime(char *s, size_t max, const char *format, const struct tm *tm);
char *strptime(const char *s, const char *format, struct tm *tm);

/* basic time conversion helpers */
struct tm *gmtime(const time_t *timep);
struct tm *localtime(const time_t *timep);
struct tm *gmtime_r(const time_t *timep, struct tm *result);
struct tm *localtime_r(const time_t *timep, struct tm *result);
void tzset(void);
time_t mktime(struct tm *tm);
time_t timegm(struct tm *tm);
char *ctime(const time_t *timep);

#endif /* TIME_H */
