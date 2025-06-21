#include "sys/file.h"
#include "errno.h"
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include "syscall.h"

int chmod(const char *path, mode_t mode)
{
    long ret = vlibc_syscall(SYS_chmod, (long)path, mode, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
}

int chown(const char *path, uid_t owner, gid_t group)
{
    long ret = vlibc_syscall(SYS_chown, (long)path, owner, group, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
}

mode_t umask(mode_t mask)
{
    long ret = vlibc_syscall(SYS_umask, mask, 0, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return (mode_t)-1;
    }
    return (mode_t)ret;
}
