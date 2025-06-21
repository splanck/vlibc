#include "sys/select.h"
#include "time.h"
#include "errno.h"
#include <sys/syscall.h>
#include <unistd.h>
#include "syscall.h"

int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
           struct timeval *timeout)
{
#ifdef SYS_select
    long ret = vlibc_syscall(SYS_select, nfds, (long)readfds, (long)writefds,
                             (long)exceptfds, (long)timeout, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
#elif defined(SYS_pselect6)
    struct timespec ts;
    struct timespec *pts = NULL;
    if (timeout) {
        ts.tv_sec = timeout->tv_sec;
        ts.tv_nsec = timeout->tv_usec * 1000;
        pts = &ts;
    }
    long ret = vlibc_syscall(SYS_pselect6, nfds, (long)readfds, (long)writefds,
                             (long)exceptfds, (long)pts, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    if (timeout && pts) {
        timeout->tv_sec = ts.tv_sec;
        timeout->tv_usec = ts.tv_nsec / 1000;
    }
    return (int)ret;
#else
    (void)nfds; (void)readfds; (void)writefds; (void)exceptfds; (void)timeout;
    errno = ENOSYS;
    return -1;
#endif
}

#include "poll.h"

int poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
#ifdef SYS_poll
    long ret = vlibc_syscall(SYS_poll, (long)fds, (long)nfds, timeout, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
#elif defined(SYS_ppoll)
    struct timespec ts;
    struct timespec *pts = NULL;
    if (timeout >= 0) {
        ts.tv_sec = timeout / 1000;
        ts.tv_nsec = (timeout % 1000) * 1000000;
        pts = &ts;
    }
    long ret = vlibc_syscall(SYS_ppoll, (long)fds, (long)nfds, (long)pts, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
#else
    (void)fds; (void)nfds; (void)timeout;
    errno = ENOSYS;
    return -1;
#endif
}
