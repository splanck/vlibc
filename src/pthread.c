#include "pthread.h"
#include_next <pthread.h>
#include <errno.h>
#include <stdatomic.h>

/* simple spinlock based mutex */
int pthread_mutex_init(pthread_mutex_t *mutex, void *attr)
{
    (void)attr;
    atomic_flag_clear(&mutex->locked);
    return 0;
}

int pthread_mutex_lock(pthread_mutex_t *mutex)
{
    while (atomic_flag_test_and_set_explicit(&mutex->locked,
                                             memory_order_acquire))
        ;
    return 0;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    atomic_flag_clear_explicit(&mutex->locked, memory_order_release);
    return 0;
}

int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
    (void)mutex;
    return 0;
}
