#ifndef PTHREAD_H
#define PTHREAD_H

#include <sys/types.h>
#include <stdatomic.h>

/*
 * vlibc threads are thin wrappers around the host pthread
 * implementation.  Keep the type compatible with glibc's
 * definition to ease interoperability.
 */
typedef unsigned long pthread_t;

typedef struct {
    atomic_flag locked;
} pthread_mutex_t;

typedef struct {
    atomic_int seq;
} pthread_cond_t;

int pthread_create(pthread_t *thread, const void *attr,
                   void *(*start_routine)(void *), void *arg);
int pthread_join(pthread_t thread, void **retval);
int pthread_detach(pthread_t thread);

int pthread_mutex_init(pthread_mutex_t *mutex, void *attr);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);

int pthread_cond_init(pthread_cond_t *cond, void *attr);
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
int pthread_cond_signal(pthread_cond_t *cond);
int pthread_cond_broadcast(pthread_cond_t *cond);

#endif /* PTHREAD_H */
