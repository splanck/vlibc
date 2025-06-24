#include "semaphore.h"
#include <errno.h>

int sem_init(sem_t *sem, int pshared, unsigned value)
{
    (void)pshared;
    if (!sem)
        return EINVAL;
    atomic_store(&sem->count, (int)value);
    return 0;
}

int sem_destroy(sem_t *sem)
{
    (void)sem;
    return 0;
}

int sem_wait(sem_t *sem)
{
    if (!sem)
        return EINVAL;
    for (;;) {
        int val = atomic_load_explicit(&sem->count, memory_order_acquire);
        if (val > 0 &&
            atomic_compare_exchange_weak_explicit(&sem->count, &val, val - 1,
                                                  memory_order_acquire,
                                                  memory_order_relaxed))
            return 0;
        struct timespec ts = {0, 1000000};
        nanosleep(&ts, NULL);
    }
}

int sem_trywait(sem_t *sem)
{
    if (!sem)
        return EINVAL;
    int val = atomic_load_explicit(&sem->count, memory_order_acquire);
    while (val > 0) {
        if (atomic_compare_exchange_weak_explicit(&sem->count, &val, val - 1,
                                                  memory_order_acquire,
                                                  memory_order_relaxed))
            return 0;
    }
    return EAGAIN;
}

int sem_post(sem_t *sem)
{
    if (!sem)
        return EINVAL;
    atomic_fetch_add_explicit(&sem->count, 1, memory_order_release);
    return 0;
}
