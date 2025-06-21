#ifndef TIME_H
#define TIME_H

#include <sys/types.h>
#include <stddef.h>
#ifndef __useconds_t_defined
typedef unsigned useconds_t;
#define __useconds_t_defined
#endif

struct timespec {
    time_t tv_sec;
    long tv_nsec;
};

struct timeval {
    time_t tv_sec;
    long tv_usec;
};

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

time_t time(time_t *t);
int gettimeofday(struct timeval *tv, void *tz);

/* sleep helpers */
unsigned sleep(unsigned seconds);
int usleep(useconds_t usec);
int nanosleep(const struct timespec *req, struct timespec *rem);

size_t strftime(char *s, size_t max, const char *format, const struct tm *tm);

/* basic time conversion helpers */
struct tm *gmtime(const time_t *timep);
struct tm *localtime(const time_t *timep);
time_t mktime(struct tm *tm);
char *ctime(const time_t *timep);

#endif /* TIME_H */
