#include "sys/stat.h"
#include "errno.h"
#include <sys/syscall.h>
#include <unistd.h>
#include "syscall.h"

#ifndef SYS_mknod
extern int host_mknod(const char *path, mode_t mode, dev_t dev) __asm__("mknod");
#endif

int mknod(const char *path, mode_t mode, dev_t dev)
{
#ifdef SYS_mknod
    long ret = vlibc_syscall(SYS_mknod, (long)path, mode, dev, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
#else
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    return host_mknod(path, mode, dev);
#else
    (void)path; (void)mode; (void)dev; errno = ENOSYS; return -1;
#endif
#endif
}
