/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms, with or without modification, are permitted provided that the copyright notice and this permission notice appear in all copies. This software is provided "as is" without warranty.
 *
 * Purpose: Implements the ttyname functions for vlibc. Provides wrappers and helpers used by the standard library.
 */

#include "unistd.h"
#include "sys/stat.h"
#include "sys/ioctl.h"
#include "string.h"
#include "errno.h"
#include <unistd.h>

#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
extern const char *devname(dev_t, mode_t);

int ttyname_r(int fd, char *buf, size_t len)
{
    if (!buf || len == 0)
        return EINVAL;

#ifdef TIOCPTYGNAME
    char tmp[128];
    if (ioctl(fd, TIOCPTYGNAME, tmp) == 0) {
        size_t l = strlen(tmp);
        if (l + 1 > len)
            return ERANGE;
        memcpy(buf, tmp, l + 1);
        return 0;
    }
#endif

    struct stat st;
    if (fstat(fd, &st) < 0)
        return errno;
    if (!S_ISCHR(st.st_mode))
        return ENOTTY;

    const char *name = devname(st.st_rdev, S_IFCHR);
    if (!name)
        return ENOENT;
    size_t l = strlen(name);
    if (l + 6 > len)
        return ERANGE;
    memcpy(buf, "/dev/", 5);
    memcpy(buf + 5, name, l + 1);
    return 0;
}

#else
extern int host_ttyname_r(int, char *, size_t) __asm("ttyname_r");

int ttyname_r(int fd, char *buf, size_t len)
{
    return host_ttyname_r(fd, buf, len);
}
#endif

char *ttyname(int fd)
{
    static __thread char namebuf[128];
    if (ttyname_r(fd, namebuf, sizeof(namebuf)) != 0)
        return NULL;
    return namebuf;
}

