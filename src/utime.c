#include "sys/file.h"
#include "errno.h"
#include <sys/syscall.h>
#include <unistd.h>
#include <fcntl.h>
#include "syscall.h"

#ifndef AT_FDCWD
#define AT_FDCWD -100
#endif

#ifndef SYS_utimensat
extern int host_utimes(const char *path, const struct timeval times[2]) __asm__("utimes");
extern int host_utime(const char *path, const struct utimbuf *times) __asm__("utime");
#endif

int utimes(const char *path, const struct timeval times[2])
{
#ifdef SYS_utimensat
    struct timespec ts[2];
    if (times) {
        ts[0].tv_sec = times[0].tv_sec;
        ts[0].tv_nsec = times[0].tv_usec * 1000;
        ts[1].tv_sec = times[1].tv_sec;
        ts[1].tv_nsec = times[1].tv_usec * 1000;
    }
    long ret = vlibc_syscall(SYS_utimensat, AT_FDCWD, (long)path,
                             times ? (long)ts : 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
#elif defined(SYS_utimes)
    long ret = vlibc_syscall(SYS_utimes, (long)path, (long)times,
                             0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
#else
    return host_utimes(path, times);
#endif
}

int utime(const char *path, const struct utimbuf *times)
{
#ifdef SYS_utimensat
    struct timespec ts[2];
    if (times) {
        ts[0].tv_sec = times->actime;
        ts[0].tv_nsec = 0;
        ts[1].tv_sec = times->modtime;
        ts[1].tv_nsec = 0;
    }
    long ret = vlibc_syscall(SYS_utimensat, AT_FDCWD, (long)path,
                             times ? (long)ts : 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
#elif defined(SYS_utime)
    long ret = vlibc_syscall(SYS_utime, (long)path, (long)times,
                             0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
#else
    return host_utime(path, times);
#endif
}
