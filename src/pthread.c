#include "pthread.h"
#include_next <pthread.h>
#include <errno.h>
#include <stdatomic.h>
#include "time.h"

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

int pthread_cond_init(pthread_cond_t *cond, void *attr)
{
    (void)attr;
    atomic_store(&cond->seq, 0);
    return 0;
}

int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
    int seq = atomic_load_explicit(&cond->seq, memory_order_relaxed);
    pthread_mutex_unlock(mutex);
    while (atomic_load_explicit(&cond->seq, memory_order_acquire) == seq) {
        struct timespec ts = {0, 1000000};
        nanosleep(&ts, NULL);
    }
    pthread_mutex_lock(mutex);
    return 0;
}

int pthread_cond_signal(pthread_cond_t *cond)
{
    atomic_fetch_add_explicit(&cond->seq, 1, memory_order_release);
    return 0;
}

int pthread_cond_broadcast(pthread_cond_t *cond)
{
    return pthread_cond_signal(cond);
}
