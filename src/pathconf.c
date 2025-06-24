#include "unistd.h"
#include "sys/statvfs.h"
#include "errno.h"
#include <limits.h>

long pathconf(const char *path, int name)
{
    struct statvfs sv;
    switch (name) {
    case _PC_NAME_MAX:
        if (statvfs(path, &sv) < 0)
            return -1;
        return (long)sv.f_namemax;
    case _PC_PATH_MAX:
#ifdef PATH_MAX
        return PATH_MAX;
#else
        return 4096;
#endif
    default:
        errno = EINVAL;
        return -1;
    }
}

long fpathconf(int fd, int name)
{
    struct statvfs sv;
    switch (name) {
    case _PC_NAME_MAX:
        if (fstatvfs(fd, &sv) < 0)
            return -1;
        return (long)sv.f_namemax;
    case _PC_PATH_MAX:
#ifdef PATH_MAX
        return PATH_MAX;
#else
        return 4096;
#endif
    default:
        errno = EINVAL;
        return -1;
    }
}
