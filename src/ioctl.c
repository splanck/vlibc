#include "sys/ioctl.h"
#include <stdarg.h>
#include "errno.h"
#include <sys/syscall.h>
#include <unistd.h>
#include "syscall.h"

int ioctl(int fd, unsigned long req, ...)
{
    va_list ap;
    va_start(ap, req);
    unsigned long arg = va_arg(ap, unsigned long);
    va_end(ap);
#ifdef SYS_ioctl
    long ret = vlibc_syscall(SYS_ioctl, fd, req, arg, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
#else
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    extern int host_ioctl(int, unsigned long, ...) __asm__("ioctl");
    return host_ioctl(fd, req, arg);
#else
    (void)fd; (void)req; (void)arg;
    errno = ENOSYS;
    return -1;
#endif
#endif
}
