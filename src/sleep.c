#include "time.h"
#include "errno.h"
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include "syscall.h"

int nanosleep(const struct timespec *req, struct timespec *rem)
{
    long ret = vlibc_syscall(SYS_nanosleep, (long)req, (long)rem, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
}

int usleep(useconds_t usec)
{
    struct timespec ts;
    ts.tv_sec = usec / 1000000;
    ts.tv_nsec = (usec % 1000000) * 1000;
    return nanosleep(&ts, NULL);
}

unsigned sleep(unsigned seconds)
{
    struct timespec ts = {seconds, 0};
    struct timespec rem = {0, 0};
    int r = nanosleep(&ts, &rem);
    if (r < 0 && errno == EINTR)
        return rem.tv_sec;
    return 0;
}

