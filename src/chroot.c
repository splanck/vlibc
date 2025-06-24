#include "unistd.h"
#include "errno.h"
#include <sys/syscall.h>
#include "syscall.h"

int chroot(const char *path)
{
#ifdef SYS_chroot
    long ret = vlibc_syscall(SYS_chroot, (long)path, 0, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
#else
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    extern int host_chroot(const char *) __asm__("chroot");
    return host_chroot(path);
#else
    (void)path;
    errno = ENOSYS;
    return -1;
#endif
#endif
}
