#ifndef PTHREAD_H
#define PTHREAD_H

#include <sys/types.h>

typedef struct pthread {
    pid_t tid;
    void *stack;
    void *(*start_routine)(void *);
    void *arg;
    void *retval;
} pthread_t;

typedef struct {
    volatile int locked;
} pthread_mutex_t;

int pthread_create(pthread_t *thread, const void *attr,
                   void *(*start_routine)(void *), void *arg);
int pthread_join(pthread_t *thread, void **retval);

int pthread_mutex_init(pthread_mutex_t *mutex, void *attr);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);

#endif /* PTHREAD_H */
