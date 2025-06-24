#include "time.h"
#include "signal.h"
#include "errno.h"
#include <sys/syscall.h>
#include <unistd.h>
#include <stdlib.h>
#include "syscall.h"
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
#include <sys/event.h>
#include <sys/time.h>
#endif

struct vlibc_timer {
#ifdef __linux__
    long id;
#else
    int kq;
    int ident;
#endif
};

int timer_create(clockid_t clockid, struct sigevent *sevp, timer_t *timerid)
{
    (void)clockid;
    if (!timerid) {
        errno = EINVAL;
        return -1;
    }
#ifdef SYS_timer_create
    struct vlibc_timer *t = malloc(sizeof(*t));
    if (!t) {
        errno = ENOMEM;
        return -1;
    }
    long id;
    long ret = vlibc_syscall(SYS_timer_create, clockid, (long)sevp, (long)&id, 0, 0, 0);
    if (ret < 0) {
        free(t);
        errno = -ret;
        return -1;
    }
    t->id = id;
    *timerid = t;
    return 0;
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
    struct vlibc_timer *t = malloc(sizeof(*t));
    if (!t) {
        errno = ENOMEM;
        return -1;
    }
    t->kq = kqueue();
    if (t->kq < 0) {
        free(t);
        return -1;
    }
    t->ident = 1;
    *timerid = t;
    (void)sevp;
    return 0;
#else
    (void)sevp;
    errno = ENOSYS;
    return -1;
#endif
}

int timer_delete(timer_t timerid)
{
    if (!timerid) {
        errno = EINVAL;
        return -1;
    }
#ifdef SYS_timer_delete
    struct vlibc_timer *t = (struct vlibc_timer *)timerid;
    long ret = vlibc_syscall(SYS_timer_delete, t->id, 0, 0, 0, 0, 0);
    int err = 0;
    if (ret < 0) {
        err = -ret;
    }
    free(t);
    if (ret < 0) {
        errno = err;
        return -1;
    }
    return 0;
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
    int r = close(timerid->kq);
    int err = errno;
    free(timerid);
    if (r < 0) {
        errno = err;
        return -1;
    }
    return 0;
#else
    (void)timerid;
    errno = ENOSYS;
    return -1;
#endif
}

int timer_settime(timer_t timerid, int flags,
                  const struct itimerspec *new_value,
                  struct itimerspec *old_value)
{
    if (!timerid || !new_value) {
        errno = EINVAL;
        return -1;
    }
#ifdef SYS_timer_settime
    struct vlibc_timer *t = (struct vlibc_timer *)timerid;
    long ret = vlibc_syscall(SYS_timer_settime, t->id, flags,
                             (long)new_value, (long)old_value, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
    struct vlibc_timer *t = (struct vlibc_timer *)timerid;
    struct kevent kev;
    int ms = new_value->it_value.tv_sec * 1000 +
             new_value->it_value.tv_nsec / 1000000;
    EV_SET(&kev, t->ident++, EVFILT_TIMER, EV_ADD | EV_ONESHOT,
           0, ms, NULL);
    (void)flags; (void)old_value;
    return kevent(t->kq, &kev, 1, NULL, 0, NULL);
#else
    (void)flags; (void)old_value;
    errno = ENOSYS;
    return -1;
#endif
}

int timer_gettime(timer_t timerid, struct itimerspec *curr_value)
{
    if (!timerid || !curr_value) {
        errno = EINVAL;
        return -1;
    }
#ifdef SYS_timer_gettime
    struct vlibc_timer *t = (struct vlibc_timer *)timerid;
    long ret = vlibc_syscall(SYS_timer_gettime, t->id,
                             (long)curr_value, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
    /* querying remaining time is not supported, return zero */
    curr_value->it_value.tv_sec = 0;
    curr_value->it_value.tv_nsec = 0;
    curr_value->it_interval.tv_sec = 0;
    curr_value->it_interval.tv_nsec = 0;
    return 0;
#else
    errno = ENOSYS;
    return -1;
#endif
}
