#include "time.h"
#include "errno.h"
#include <sys/syscall.h>
#include <unistd.h>
#include "syscall.h"

int clock_gettime(int clk_id, struct timespec *ts)
{
#ifdef SYS_clock_gettime
    long ret = vlibc_syscall(SYS_clock_gettime, clk_id, (long)ts, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
#else
    struct timeval tv;
    if (gettimeofday(&tv, NULL) < 0)
        return -1;
    if (ts) {
        ts->tv_sec = tv.tv_sec;
        ts->tv_nsec = tv.tv_usec * 1000;
    }
    return 0;
#endif
}
