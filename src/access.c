#include "io.h"
#include "errno.h"
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include "syscall.h"

int access(const char *pathname, int mode)
{
#ifdef SYS_access
    long ret = vlibc_syscall(SYS_access, (long)pathname, mode, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
#else
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    extern int host_access(const char *, int) __asm__("access");
    return host_access(pathname, mode);
#else
    (void)pathname; (void)mode; errno = ENOSYS; return -1;
#endif
#endif
}

int faccessat(int dirfd, const char *pathname, int mode, int flags)
{
#ifdef SYS_faccessat
    long ret = vlibc_syscall(SYS_faccessat, dirfd, (long)pathname, mode, flags, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
#else
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    extern int host_faccessat(int, const char *, int, int) __asm__("faccessat");
    return host_faccessat(dirfd, pathname, mode, flags);
#else
    (void)dirfd; (void)pathname; (void)mode; (void)flags; errno = ENOSYS; return -1;
#endif
#endif
}
