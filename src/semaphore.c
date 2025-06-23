#include "semaphore.h"
#include "errno.h"
#include "time.h"
#include <stdatomic.h>

#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)

/* use host pthread semaphore implementation */
extern int host_sem_init(sem_t *, int, unsigned) __asm("sem_init");
extern int host_sem_destroy(sem_t *) __asm("sem_destroy");
extern int host_sem_wait(sem_t *) __asm("sem_wait");
extern int host_sem_post(sem_t *) __asm("sem_post");
extern int host_sem_trywait(sem_t *) __asm("sem_trywait");

int sem_init(sem_t *sem, int pshared, unsigned value)
{
    return host_sem_init(sem, pshared, value);
}

int sem_destroy(sem_t *sem)
{
    return host_sem_destroy(sem);
}

int sem_wait(sem_t *sem)
{
    return host_sem_wait(sem);
}

int sem_trywait(sem_t *sem)
{
    return host_sem_trywait(sem);
}

int sem_post(sem_t *sem)
{
    return host_sem_post(sem);
}

#else

int sem_init(sem_t *sem, int pshared, unsigned value)
{
    (void)pshared;
    if (!sem)
        return -1;
    atomic_store(&sem->value, (int)value);
    return 0;
}

int sem_destroy(sem_t *sem)
{
    (void)sem;
    return 0;
}

int sem_wait(sem_t *sem)
{
    while (1) {
        int val = atomic_load_explicit(&sem->value, memory_order_acquire);
        while (val <= 0) {
            struct timespec ts = {0, 1000000};
            nanosleep(&ts, NULL);
            val = atomic_load_explicit(&sem->value, memory_order_acquire);
        }
        if (atomic_compare_exchange_weak_explicit(&sem->value, &val, val - 1,
                                                  memory_order_acquire,
                                                  memory_order_relaxed))
            return 0;
    }
}

int sem_trywait(sem_t *sem)
{
    int val = atomic_load_explicit(&sem->value, memory_order_acquire);
    while (val > 0) {
        if (atomic_compare_exchange_weak_explicit(&sem->value, &val, val - 1,
                                                  memory_order_acquire,
                                                  memory_order_relaxed))
            return 0;
    }
    errno = EAGAIN;
    return -1;
}

int sem_post(sem_t *sem)
{
    atomic_fetch_add_explicit(&sem->value, 1, memory_order_release);
    return 0;
}

#endif
