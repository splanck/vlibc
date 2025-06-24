/*
 * BSD 2-Clause License
 *
 * Purpose: Declarations for minimal threading support.
 */
#ifndef PTHREAD_H
#define PTHREAD_H

#include <sys/types.h>
#include <stdatomic.h>
#include "time.h"

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
    int type;
} pthread_mutexattr_t;

#define PTHREAD_MUTEX_NORMAL 0
#define PTHREAD_MUTEX_RECURSIVE 1

typedef struct {
    atomic_int seq;
} pthread_cond_t;

typedef struct {
    atomic_int readers;
    atomic_int writer;
} pthread_rwlock_t;

typedef unsigned int pthread_key_t;

typedef struct {
    atomic_int done;
} pthread_once_t;

#define PTHREAD_ONCE_INIT { ATOMIC_VAR_INIT(0) }

int pthread_create(pthread_t *thread, const void *attr,
                   void *(*start_routine)(void *), void *arg);
int pthread_join(pthread_t thread, void **retval);
int pthread_detach(pthread_t thread);
pthread_t pthread_self(void);
int pthread_equal(pthread_t a, pthread_t b);

int pthread_mutex_init(pthread_mutex_t *mutex, void *attr);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_trylock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);

int pthread_mutexattr_init(pthread_mutexattr_t *attr);
int pthread_mutexattr_destroy(pthread_mutexattr_t *attr);
int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type);
int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *type);

int pthread_cond_init(pthread_cond_t *cond, void *attr);
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex,
                           const struct timespec *abstime);
int pthread_cond_signal(pthread_cond_t *cond);
int pthread_cond_broadcast(pthread_cond_t *cond);

int pthread_rwlock_init(pthread_rwlock_t *rwlock, void *attr);
int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock);
int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock);
int pthread_rwlock_unlock(pthread_rwlock_t *rwlock);
int pthread_rwlock_destroy(pthread_rwlock_t *rwlock);

int pthread_key_create(pthread_key_t *key, void (*destructor)(void *));
int pthread_key_delete(pthread_key_t key);
int pthread_setspecific(pthread_key_t key, const void *value);
void *pthread_getspecific(pthread_key_t key);

int pthread_once(pthread_once_t *once_control, void (*init_routine)(void));

#endif /* PTHREAD_H */
