#define _GNU_SOURCE
#include <dirent.h>
#include "dirent.h"
#undef opendir
#undef readdir
#undef closedir
#include "memory.h"
#include "io.h"
#include "errno.h"
#include <string.h>
#include <fcntl.h>
#ifndef O_DIRECTORY
#define O_DIRECTORY 0200000
#endif

typedef struct __dirstream sys_DIR;

/* Declare the system directory functions we wrap. */
extern DIR *fdopendir(int);
extern struct dirent *readdir(sys_DIR *);
extern int closedir(sys_DIR *);

DIR *vlibc_opendir(const char *name)
{
    int fd = open(name, O_RDONLY | O_DIRECTORY);
    if (fd < 0)
        return NULL;

    DIR *d = malloc(sizeof(DIR));
    if (!d) {
        close(fd);
        errno = ENOMEM;
        return NULL;
    }
    d->impl = fdopendir(fd);
    if (!d->impl) {
        close(fd);
        free(d);
        return NULL;
    }
    return d;
}

struct dirent *vlibc_readdir(DIR *dirp)
{
    if (!dirp || !dirp->impl)
        return NULL;

    struct dirent *e = readdir((sys_DIR *)dirp->impl);
    if (!e)
        return NULL;
    memcpy(&dirp->ent, e, sizeof(struct dirent));
    return &dirp->ent;
}

int vlibc_closedir(DIR *dirp)
{
    if (!dirp)
        return -1;
    int r = closedir((sys_DIR *)dirp->impl);
    free(dirp);
    return r;
}
