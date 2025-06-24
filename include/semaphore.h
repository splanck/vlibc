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
int sem_destroy(sem_t *sem);
int sem_wait(sem_t *sem);
int sem_trywait(sem_t *sem);
int sem_post(sem_t *sem);

#endif /* SEMAPHORE_H */
