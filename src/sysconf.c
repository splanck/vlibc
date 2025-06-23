#include "unistd.h"
#include "errno.h"

#include <sys/types.h>
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
#include <sys/sysctl.h>
#include <sys/resource.h>
#include <unistd.h>
#endif

long sysconf(int name)
{
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    size_t len;
    int mib[2];
    switch (name) {
    case _SC_PAGESIZE:
    case _SC_PAGE_SIZE: {
        int page;
        mib[0] = CTL_HW; mib[1] = HW_PAGESIZE;
        len = sizeof(page);
        if (sysctl(mib, 2, &page, &len, NULL, 0) == 0)
            return page;
        break;
    }
    case _SC_OPEN_MAX: {
        struct rlimit rl;
        if (getrlimit(RLIMIT_NOFILE, &rl) == 0)
            return (long)rl.rlim_cur;
        break;
    }
    case _SC_NPROCESSORS_ONLN: {
        int ncpu;
        mib[0] = CTL_HW; mib[1] = HW_NCPU;
        len = sizeof(ncpu);
        if (sysctl(mib, 2, &ncpu, &len, NULL, 0) == 0)
            return ncpu;
        break;
    }
    case _SC_ARG_MAX: {
        int argmax;
        mib[0] = CTL_KERN; mib[1] = KERN_ARGMAX;
        len = sizeof(argmax);
        if (sysctl(mib, 2, &argmax, &len, NULL, 0) == 0)
            return argmax;
        break;
    }
    case _SC_CLK_TCK: {
#ifdef KERN_CLOCKRATE
        struct clockinfo ci;
        mib[0] = CTL_KERN; mib[1] = KERN_CLOCKRATE;
        len = sizeof(ci);
        if (sysctl(mib, 2, &ci, &len, NULL, 0) == 0)
            return ci.hz;
#endif
        break;
    }
    default:
        errno = EINVAL;
        return -1;
    }
    errno = EINVAL;
    return -1;
#else
    extern long host_sysconf(int) __asm("sysconf");
    return host_sysconf(name);
#endif
}
