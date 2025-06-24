/*
 * BSD 2-Clause License
 *
 * Purpose: Declarations for counting semaphore primitives.
 */
#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <stdatomic.h>
#include "time.h"

typedef struct {
    atomic_int count;
} sem_t;

int sem_init(sem_t *sem, int pshared, unsigned value);
/* Initialize a counting semaphore with the given starting value. */
int sem_destroy(sem_t *sem);
/* Destroy the semaphore object (no-op for this implementation). */
int sem_wait(sem_t *sem);
/* Decrement the semaphore count, blocking until it becomes positive. */
int sem_trywait(sem_t *sem);
/* Try to decrement the semaphore without blocking. */
int sem_post(sem_t *sem);
/* Increment the semaphore count and wake any waiting threads. */

#endif /* SEMAPHORE_H */
