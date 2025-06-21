#ifndef TIME_H
#define TIME_H

#include <sys/types.h>
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

time_t time(time_t *t);
int gettimeofday(struct timeval *tv, void *tz);

/* sleep helpers */
unsigned sleep(unsigned seconds);
int usleep(useconds_t usec);
int nanosleep(const struct timespec *req, struct timespec *rem);

#endif /* TIME_H */
