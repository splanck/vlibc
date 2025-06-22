#include "pthread.h"
#include_next <pthread.h>
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
