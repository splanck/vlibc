/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms, with
 * or without modification, are permitted provided that the copyright notice and this permission notice appear in all copies. This software is provided "as is" without warranty.
 *
 * Purpose: Implements POSIX message queue wrappers for vlibc.
 */

#include "mqueue.h"
#include "errno.h"
#include <sys/syscall.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include "syscall.h"

mqd_t mq_open(const char *name, int oflag, ...)
{
    mode_t mode = 0;
    struct mq_attr *attr = NULL;
    if (oflag & O_CREAT) {
        va_list ap;
        va_start(ap, oflag);
        mode = va_arg(ap, mode_t);
        attr = va_arg(ap, struct mq_attr *);
        va_end(ap);
    }
#ifdef SYS_mq_open
    long ret = vlibc_syscall(SYS_mq_open, (long)name, oflag, mode, (long)attr, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return (mqd_t)-1;
    }
    return (mqd_t)ret;
#elif defined(__FreeBSD__) || defined(__NetBSD__) || \
      defined(__OpenBSD__) || defined(__DragonFly__)
    extern mqd_t host_mq_open(const char *, int, ...) __asm__("mq_open");
    if (oflag & O_CREAT)
        return host_mq_open(name, oflag, mode, attr);
    return host_mq_open(name, oflag);
#else
    (void)name; (void)oflag; (void)mode; (void)attr;
    errno = ENOSYS;
    return (mqd_t)-1;
#endif
}

int mq_close(mqd_t mqdes)
{
#ifdef SYS_mq_close
    long ret = vlibc_syscall(SYS_mq_close, mqdes, 0, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
#elif defined(__FreeBSD__) || defined(__NetBSD__) || \
      defined(__OpenBSD__) || defined(__DragonFly__)
    extern int host_mq_close(mqd_t) __asm__("mq_close");
    return host_mq_close(mqdes);
#else
    (void)mqdes;
    errno = ENOSYS;
    return -1;
#endif
}

int mq_unlink(const char *name)
{
#ifdef SYS_mq_unlink
    long ret = vlibc_syscall(SYS_mq_unlink, (long)name, 0, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
#elif defined(__FreeBSD__) || defined(__NetBSD__) || \
      defined(__OpenBSD__) || defined(__DragonFly__)
    extern int host_mq_unlink(const char *) __asm__("mq_unlink");
    return host_mq_unlink(name);
#else
    (void)name;
    errno = ENOSYS;
    return -1;
#endif
}

int mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned msg_prio)
{
#ifdef SYS_mq_send
    long ret = vlibc_syscall(SYS_mq_send, mqdes, (long)msg_ptr, msg_len, msg_prio, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
#elif defined(__FreeBSD__) || defined(__NetBSD__) || \
      defined(__OpenBSD__) || defined(__DragonFly__)
    extern int host_mq_send(mqd_t, const char *, size_t, unsigned) __asm__("mq_send");
    return host_mq_send(mqdes, msg_ptr, msg_len, msg_prio);
#else
    (void)mqdes; (void)msg_ptr; (void)msg_len; (void)msg_prio;
    errno = ENOSYS;
    return -1;
#endif
}

ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned *msg_prio)
{
#ifdef SYS_mq_receive
    long ret = vlibc_syscall(SYS_mq_receive, mqdes, (long)msg_ptr, msg_len, (long)msg_prio, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (ssize_t)ret;
#elif defined(__FreeBSD__) || defined(__NetBSD__) || \
      defined(__OpenBSD__) || defined(__DragonFly__)
    extern ssize_t host_mq_receive(mqd_t, char *, size_t, unsigned *) __asm__("mq_receive");
    return host_mq_receive(mqdes, msg_ptr, msg_len, msg_prio);
#else
    (void)mqdes; (void)msg_ptr; (void)msg_len; (void)msg_prio;
    errno = ENOSYS;
    return -1;
#endif
}
