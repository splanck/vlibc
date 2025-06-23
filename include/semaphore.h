#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <sys/types.h>

#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
#  if defined(__has_include)
#    if __has_include_next(<semaphore.h>)
#      include_next <semaphore.h>
#      define VLIBC_SEM_NATIVE 1
#    endif
#  endif
#endif

#ifndef VLIBC_SEM_NATIVE
#include <stdatomic.h>

typedef struct {
    atomic_int value;
} sem_t;
#endif

int sem_init(sem_t *sem, int pshared, unsigned value);
int sem_destroy(sem_t *sem);
int sem_wait(sem_t *sem);
int sem_post(sem_t *sem);
int sem_trywait(sem_t *sem);

#endif /* SEMAPHORE_H */
