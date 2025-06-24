#include "pthread.h"
#include_next <pthread.h>
#include <errno.h>
#include <stdatomic.h>
#include "time.h"

extern pthread_t host_pthread_self(void) __asm__("pthread_self");
extern int host_pthread_equal(pthread_t, pthread_t) __asm__("pthread_equal");

/* simple spinlock based mutex */
int pthread_mutex_init(pthread_mutex_t *mutex, void *attr)
{
    (void)attr;
    atomic_flag_clear(&mutex->locked);
    return 0;
}

int pthread_mutex_lock(pthread_mutex_t *mutex)
{
    while (atomic_flag_test_and_set_explicit(&mutex->locked,
                                             memory_order_acquire))
        ;
    return 0;
}

int pthread_mutex_trylock(pthread_mutex_t *mutex)
{
    if (atomic_flag_test_and_set_explicit(&mutex->locked,
                                          memory_order_acquire))
        return EBUSY;
    return 0;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    atomic_flag_clear_explicit(&mutex->locked, memory_order_release);
    return 0;
}

int pthread_mutexattr_init(pthread_mutexattr_t *attr)
{
    if (!attr)
        return EINVAL;
    attr->type = PTHREAD_MUTEX_NORMAL;
    return 0;
}

int pthread_mutexattr_destroy(pthread_mutexattr_t *attr)
{
    (void)attr;
    return 0;
}

int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type)
{
    if (!attr)
        return EINVAL;
    if (type != PTHREAD_MUTEX_NORMAL && type != PTHREAD_MUTEX_RECURSIVE)
        return EINVAL;
    attr->type = type;
    return 0;
}

int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *type)
{
    if (!attr || !type)
        return EINVAL;
    *type = attr->type;
    return 0;
}

int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
    (void)mutex;
    return 0;
}

int pthread_cond_init(pthread_cond_t *cond, void *attr)
{
    (void)attr;
    atomic_store(&cond->seq, 0);
    return 0;
}

int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
    int seq = atomic_load_explicit(&cond->seq, memory_order_relaxed);
    pthread_mutex_unlock(mutex);
    while (atomic_load_explicit(&cond->seq, memory_order_acquire) == seq) {
        struct timespec ts = {0, 1000000};
        nanosleep(&ts, NULL);
    }
    pthread_mutex_lock(mutex);
    return 0;
}

int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex,
                           const struct timespec *abstime)
{
    if (!abstime)
        return pthread_cond_wait(cond, mutex);

    int seq = atomic_load_explicit(&cond->seq, memory_order_relaxed);
    pthread_mutex_unlock(mutex);

    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    while (atomic_load_explicit(&cond->seq, memory_order_acquire) == seq) {
        if (now.tv_sec > abstime->tv_sec ||
            (now.tv_sec == abstime->tv_sec &&
             now.tv_nsec >= abstime->tv_nsec)) {
            pthread_mutex_lock(mutex);
            return ETIMEDOUT;
        }
        struct timespec ts = {0, 1000000};
        nanosleep(&ts, NULL);
        clock_gettime(CLOCK_REALTIME, &now);
    }

    pthread_mutex_lock(mutex);
    return 0;
}

int pthread_cond_signal(pthread_cond_t *cond)
{
    atomic_fetch_add_explicit(&cond->seq, 1, memory_order_release);
    return 0;
}

int pthread_cond_broadcast(pthread_cond_t *cond)
{
    return pthread_cond_signal(cond);
}

#define KEY_MAX 64

static pthread_mutex_t key_lock;
static void (*key_destructors[KEY_MAX])(void *);
static __thread void *key_values[KEY_MAX];

int pthread_key_create(pthread_key_t *key, void (*destructor)(void *))
{
    if (!key)
        return EINVAL;
    pthread_mutex_lock(&key_lock);
    for (unsigned i = 0; i < KEY_MAX; ++i) {
        if (!key_destructors[i]) {
            key_destructors[i] = destructor ? destructor : (void *)1;
            *key = i;
            pthread_mutex_unlock(&key_lock);
            return 0;
        }
    }
    pthread_mutex_unlock(&key_lock);
    return EAGAIN;
}

int pthread_key_delete(pthread_key_t key)
{
    if (key >= KEY_MAX)
        return EINVAL;
    pthread_mutex_lock(&key_lock);
    key_destructors[key] = NULL;
    key_values[key] = NULL;
    pthread_mutex_unlock(&key_lock);
    return 0;
}

int pthread_setspecific(pthread_key_t key, const void *value)
{
    if (key >= KEY_MAX || !key_destructors[key])
        return EINVAL;
    key_values[key] = (void *)value;
    return 0;
}

void *pthread_getspecific(pthread_key_t key)
{
    if (key >= KEY_MAX || !key_destructors[key])
        return NULL;
    return key_values[key];
}

static pthread_mutex_t once_lock;

int pthread_once(pthread_once_t *once_control, void (*init_routine)(void))
{
    if (atomic_load_explicit(&once_control->done, memory_order_acquire))
        return 0;
    pthread_mutex_lock(&once_lock);
    if (!atomic_load_explicit(&once_control->done, memory_order_acquire)) {
        init_routine();
        atomic_store_explicit(&once_control->done, 1, memory_order_release);
    }
    pthread_mutex_unlock(&once_lock);
    return 0;
}

pthread_t pthread_self(void)
{
    return host_pthread_self();
}

int pthread_equal(pthread_t a, pthread_t b)
{
    return host_pthread_equal(a, b);
}
