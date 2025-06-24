#include "sys/file.h"
#include "errno.h"
#include <sys/syscall.h>
#include <unistd.h>
#include "syscall.h"

int flock(int fd, int operation)
{
#ifdef SYS_flock
    long ret = vlibc_syscall(SYS_flock, fd, operation, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
#else
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    extern int host_flock(int, int) __asm__("flock");
    return host_flock(fd, operation);
#else
    (void)fd; (void)operation; errno = ENOSYS; return -1;
#endif
#endif
}
