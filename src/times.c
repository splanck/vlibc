/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms, with or without modification, are permitted provided that the copyright notice and this permission notice appear in all copies. This software is provided "as is" without warranty.
 *
 * Purpose: Implements the times wrapper for vlibc. Provides process CPU time information.
 */

#include "sys/times.h"
#include "errno.h"
#include <sys/syscall.h>
#include <unistd.h>
#include "syscall.h"

clock_t times(struct tms *buf)
{
#ifdef SYS_times
    long ret = vlibc_syscall(SYS_times, (long)buf, 0, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return (clock_t)-1;
    }
    return (clock_t)ret;
#elif defined(__FreeBSD__) || defined(__NetBSD__) || \
      defined(__OpenBSD__) || defined(__DragonFly__)
    extern clock_t host_times(struct tms *) __asm("times");
    return host_times(buf);
#else
    (void)buf;
    errno = ENOSYS;
    return (clock_t)-1;
#endif
}
