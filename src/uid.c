#include "unistd.h"
#include "errno.h"
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include "syscall.h"

#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
extern uid_t host_getuid(void) __asm("getuid");
extern uid_t host_geteuid(void) __asm("geteuid");
extern gid_t host_getgid(void) __asm("getgid");
extern gid_t host_getegid(void) __asm("getegid");
#endif

uid_t getuid(void)
{
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    return host_getuid();
#else
    long ret = vlibc_syscall(SYS_getuid, 0, 0, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return (uid_t)-1;
    }
    return (uid_t)ret;
#endif
}

uid_t geteuid(void)
{
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    return host_geteuid();
#else
    long ret = vlibc_syscall(SYS_geteuid, 0, 0, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return (uid_t)-1;
    }
    return (uid_t)ret;
#endif
}

gid_t getgid(void)
{
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    return host_getgid();
#else
    long ret = vlibc_syscall(SYS_getgid, 0, 0, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return (gid_t)-1;
    }
    return (gid_t)ret;
#endif
}

gid_t getegid(void)
{
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    return host_getegid();
#else
    long ret = vlibc_syscall(SYS_getegid, 0, 0, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return (gid_t)-1;
    }
    return (gid_t)ret;
#endif
}
