#include "env.h"
#include "errno.h"
#include <sys/syscall.h>
#include <unistd.h>
#include "syscall.h"

int gethostname(char *name, size_t len)
{
#ifdef SYS_gethostname
    long ret = vlibc_syscall(SYS_gethostname, (long)name, len, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
#else
    extern int host_gethostname(char *, size_t) __asm__("gethostname");
    return host_gethostname(name, len);
#endif
}

int sethostname(const char *name, size_t len)
{
#ifdef SYS_sethostname
    long ret = vlibc_syscall(SYS_sethostname, (long)name, len, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
#else
    extern int host_sethostname(const char *, size_t) __asm__("sethostname");
    return host_sethostname(name, len);
#endif
}
