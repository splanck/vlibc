#include "pthread.h"
#include <errno.h>

/* simple spinlock based mutex */
int pthread_mutex_init(pthread_mutex_t *mutex, void *attr)
{
    (void)attr;
    mutex->locked = 0;
    return 0;
}

int pthread_mutex_lock(pthread_mutex_t *mutex)
{
    while (__sync_lock_test_and_set(&mutex->locked, 1))
        ;
    return 0;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    __sync_lock_release(&mutex->locked);
    return 0;
}

int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
    (void)mutex;
    return 0;
}

/* wrappers around host pthread implementation */
extern int host_pthread_create(pthread_t *t, const void *a,
                               void *(*fn)(void *), void *arg)
    __asm__("pthread_create");
extern int host_pthread_join(pthread_t t, void **ret)
    __asm__("pthread_join");
extern int host_pthread_detach(pthread_t t)
    __asm__("pthread_detach");

int pthread_create(pthread_t *thread, const void *attr,
                   void *(*start_routine)(void *), void *arg)
{
    int r = host_pthread_create(thread, attr, start_routine, arg);
    if (r != 0) {
        errno = r;
        return -1;
    }
    return 0;
}

int pthread_join(pthread_t thread, void **retval)
{
    int r = host_pthread_join(thread, retval);
    if (r != 0) {
        errno = r;
        return -1;
    }
    return 0;
}

int pthread_detach(pthread_t thread)
{
    int r = host_pthread_detach(thread);
    if (r != 0) {
        errno = r;
        return -1;
    }
    return 0;
}
