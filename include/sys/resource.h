#ifndef SYS_RESOURCE_H
#define SYS_RESOURCE_H

#include <sys/types.h>

/* Resource limits */
#ifndef RLIMIT_CPU
#define RLIMIT_CPU        0
#endif
#ifndef RLIMIT_FSIZE
#define RLIMIT_FSIZE      1
#endif
#ifndef RLIMIT_DATA
#define RLIMIT_DATA       2
#endif
#ifndef RLIMIT_STACK
#define RLIMIT_STACK      3
#endif
#ifndef RLIMIT_CORE
#define RLIMIT_CORE       4
#endif
#ifndef RLIMIT_RSS
#define RLIMIT_RSS        5
#endif
#ifndef RLIMIT_NPROC
#define RLIMIT_NPROC      6
#endif
#ifndef RLIMIT_NOFILE
#define RLIMIT_NOFILE     7
#endif
#ifndef RLIMIT_MEMLOCK
#define RLIMIT_MEMLOCK    8
#endif
#ifndef RLIMIT_AS
#define RLIMIT_AS         9
#endif
#ifndef RLIMIT_LOCKS
#define RLIMIT_LOCKS      10
#endif
#ifndef RLIMIT_SIGPENDING
#define RLIMIT_SIGPENDING 11
#endif
#ifndef RLIMIT_MSGQUEUE
#define RLIMIT_MSGQUEUE   12
#endif
#ifndef RLIMIT_NICE
#define RLIMIT_NICE       13
#endif
#ifndef RLIMIT_RTPRIO
#define RLIMIT_RTPRIO     14
#endif
#ifndef RLIMIT_RTTIME
#define RLIMIT_RTTIME     15
#endif
#ifndef RLIMIT_NLIMITS
#define RLIMIT_NLIMITS    16
#endif

#ifndef RLIMIT_OFILE
#define RLIMIT_OFILE RLIMIT_NOFILE
#endif

/* Infinite resource limit */
#ifndef RLIM_INFINITY
#define RLIM_INFINITY ((rlim_t)-1)
#endif

/* Type for resource limits */
typedef unsigned long rlim_t;

struct rlimit {
    rlim_t rlim_cur;
    rlim_t rlim_max;
};

int getrlimit(int resource, struct rlimit *rlim);
int setrlimit(int resource, const struct rlimit *rlim);

#endif /* SYS_RESOURCE_H */
