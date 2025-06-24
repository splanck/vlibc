#include "time.h"
#include "errno.h"
#include <sys/syscall.h>
#include <unistd.h>
#include "syscall.h"

int clock_getres(int clk_id, struct timespec *res)
{
#ifdef SYS_clock_getres
    long ret = vlibc_syscall(SYS_clock_getres, clk_id, (long)res, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
#elif defined(__FreeBSD__) || defined(__NetBSD__) || \
      defined(__OpenBSD__) || defined(__DragonFly__)
    extern int host_clock_getres(int, struct timespec *) __asm__("clock_getres");
    return host_clock_getres(clk_id, res);
#else
    (void)clk_id; (void)res;
    errno = ENOSYS;
    return -1;
#endif
}
