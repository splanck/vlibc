/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms, with or without modification, are permitted provided that the copyright notice and this permission notice appear in all copies. This software is provided "as is" without warranty.
 *
 * Purpose: Implements the pthread_barrier functions for vlibc.
 */

#include "pthread.h"
#include <stdatomic.h>
#include <errno.h>

int pthread_barrier_init(pthread_barrier_t *barrier, void *attr, unsigned count)
{
    (void)attr;
    if (!barrier || count == 0)
        return EINVAL;
    barrier->count = count;
    atomic_store(&barrier->waiting, 0);
    pthread_mutex_init(&barrier->lock, NULL);
    pthread_cond_init(&barrier->cond, NULL);
    return 0;
}

int pthread_barrier_wait(pthread_barrier_t *barrier)
{
    pthread_mutex_lock(&barrier->lock);
    unsigned w = atomic_fetch_add_explicit(&barrier->waiting, 1,
                                           memory_order_acquire) + 1;
    if (w == barrier->count) {
        atomic_store_explicit(&barrier->waiting, 0, memory_order_release);
        pthread_cond_broadcast(&barrier->cond);
        pthread_mutex_unlock(&barrier->lock);
        return PTHREAD_BARRIER_SERIAL_THREAD;
    }
    while (atomic_load_explicit(&barrier->waiting, memory_order_acquire) != 0)
        pthread_cond_wait(&barrier->cond, &barrier->lock);
    pthread_mutex_unlock(&barrier->lock);
    return 0;
}

int pthread_barrier_destroy(pthread_barrier_t *barrier)
{
    (void)barrier;
    return 0;
}

