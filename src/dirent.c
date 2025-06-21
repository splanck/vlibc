#include "dirent.h"
#include "memory.h"
#include "io.h"
#include "errno.h"
#include "string.h"
#include <fcntl.h>
#ifndef O_DIRECTORY
#define O_DIRECTORY 0200000
#endif
#include <sys/syscall.h>
#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include "syscall.h"

struct linux_dirent64 {
    uint64_t d_ino;
    int64_t  d_off;
    unsigned short d_reclen;
    unsigned char  d_type;
    char d_name[];
};

DIR *opendir(const char *name)
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
    d->fd = fd;
    d->buf_pos = 0;
    d->buf_len = 0;
    return d;
}

struct dirent *readdir(DIR *dirp)
{
    if (!dirp)
        return NULL;

    if (dirp->buf_pos >= dirp->buf_len) {
        long ret = vlibc_syscall(SYS_getdents64, dirp->fd,
                                 (long)dirp->buf, sizeof(dirp->buf), 0, 0, 0);
        if (ret < 0) {
            errno = -ret;
            return NULL;
        }
        if (ret == 0)
            return NULL;
        dirp->buf_len = (size_t)ret;
        dirp->buf_pos = 0;
    }

    struct linux_dirent64 *ld = (struct linux_dirent64 *)(dirp->buf + dirp->buf_pos);
    dirp->buf_pos += ld->d_reclen;

    dirp->ent.d_ino = (ino_t)ld->d_ino;
    dirp->ent.d_off = (off_t)ld->d_off;
    dirp->ent.d_reclen = ld->d_reclen;
    dirp->ent.d_type = ld->d_type;

    size_t name_len = ld->d_reclen - offsetof(struct linux_dirent64, d_name);
    if (name_len >= sizeof(dirp->ent.d_name))
        name_len = sizeof(dirp->ent.d_name) - 1;
    vmemcpy(dirp->ent.d_name, ld->d_name, name_len);
    dirp->ent.d_name[name_len] = '\0';

    return &dirp->ent;
}

int closedir(DIR *dirp)
{
    if (!dirp)
        return -1;
    int r = close(dirp->fd);
    free(dirp);
    return r;
}

