#include "unistd.h"
#include "io.h"
#include "sys/file.h"
#include "errno.h"
#include <sys/syscall.h>
#include "syscall.h"

int daemon(int nochdir, int noclose)
{
    pid_t pid = fork();
    if (pid < 0)
        return -1;
    if (pid > 0)
        _exit(0);

    long ret;
#ifdef SYS_setsid
    ret = vlibc_syscall(SYS_setsid, 0, 0, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
#else
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    extern pid_t host_setsid(void) __asm__("setsid");
    if (host_setsid() < 0)
        return -1;
#else
    errno = ENOSYS;
    return -1;
#endif
#endif

    if (!nochdir)
        chdir("/");
    umask(0);

    if (!noclose) {
        int fd = open("/dev/null", 2);
        if (fd < 0)
            fd = open("/dev/null", 0);
        if (fd >= 0) {
            dup2(fd, 0);
            dup2(fd, 1);
            dup2(fd, 2);
            if (fd > 2)
                close(fd);
        }
    }

    return 0;
}
