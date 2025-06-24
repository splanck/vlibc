#include "sys/resource.h"
#include "errno.h"
#include <sys/syscall.h>
#include <unistd.h>
#include "syscall.h"

int getrusage(int who, struct rusage *usage)
{
#if defined(SYS_getrusage)
    long ret = vlibc_syscall(SYS_getrusage, who, (long)usage, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
#elif defined(__FreeBSD__) || defined(__NetBSD__) || \
      defined(__OpenBSD__) || defined(__DragonFly__)
    extern int host_getrusage(int, struct rusage *) __asm("getrusage");
    return host_getrusage(who, usage);
#else
    (void)who; (void)usage;
    errno = ENOSYS;
    return -1;
#endif
}
