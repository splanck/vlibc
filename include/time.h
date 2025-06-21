#ifndef TIME_H
#define TIME_H

#include <sys/types.h>

struct timespec {
    time_t tv_sec;
    long tv_nsec;
};

struct timeval {
    time_t tv_sec;
    long tv_usec;
};

time_t time(time_t *t);
int gettimeofday(struct timeval *tv, void *tz);

#endif /* TIME_H */
