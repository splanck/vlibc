/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms, with
 * or without modification, are permitted provided that the copyright notice and this
 * permission notice appear in all copies. This software is provided "as is" without warranty.
 *
 * Purpose: Implements the pthread spin lock functions for vlibc.
 */

#include "pthread.h"
#include <errno.h>
#include <stdatomic.h>
#include "futex.h"

/* Initialize a spin lock object. Only process-private locks are supported. */
int pthread_spin_init(pthread_spinlock_t *lock, int pshared)
{
    (void)pshared;
    if (!lock)
        return EINVAL;
    atomic_store(&lock->locked, 0);
    return 0;
}

/* Acquire the spin lock, busy-waiting until available. */
int pthread_spin_lock(pthread_spinlock_t *lock)
{
    if (!lock)
        return EINVAL;
    while (atomic_exchange_explicit(&lock->locked, 1, memory_order_acquire))
        futex_wait(&lock->locked, 1, NULL);
    return 0;
}

/* Try to acquire the spin lock without blocking. */
int pthread_spin_trylock(pthread_spinlock_t *lock)
{
    if (!lock)
        return EINVAL;
    int expected = 0;
    if (!atomic_compare_exchange_strong_explicit(&lock->locked, &expected, 1,
                                                 memory_order_acquire,
                                                 memory_order_relaxed))
        return EBUSY;
    return 0;
}

/* Release the spin lock. */
int pthread_spin_unlock(pthread_spinlock_t *lock)
{
    if (!lock)
        return EINVAL;
    atomic_store_explicit(&lock->locked, 0, memory_order_release);
    futex_wake(&lock->locked, 1);
    return 0;
}

/* Destroy a spin lock object (no-op). */
int pthread_spin_destroy(pthread_spinlock_t *lock)
{
    (void)lock;
    return 0;
}
