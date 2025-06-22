#include "pthread.h"
#include "memory.h"
#include "process.h"
#include "errno.h"
#include "syscall.h"
#include <sys/syscall.h>
#include <stdint.h>
#include <stddef.h>

int clone(int (*fn)(void *), void *child_stack, int flags, void *arg);

#define CLONE_VM        0x00000100
#define CLONE_FS        0x00000200
#define CLONE_FILES     0x00000400
#define CLONE_SIGHAND   0x00000800

/* very small stack for demo */
#define STACK_SIZE (64 * 1024)

struct start_info {
    pthread_t *thread;
    void *(*func)(void *);
    void *arg;
};

static struct start_info g_info;

static int start_thread(void *arg)
{
    struct start_info *info = arg;
    void *rv = info->func(info->arg);
    info->thread->retval = rv;
    if (info->thread->detached)
        free(info->thread->stack);
    _exit(0);
    return 0;
}

int pthread_create(pthread_t *thread, const void *attr,
                   void *(*start_routine)(void *), void *arg)
{
    (void)attr;

    thread->start_routine = start_routine;
    thread->arg = arg;
    thread->retval = NULL;
    thread->detached = 0;
    thread->stack = malloc(STACK_SIZE + 16);
    if (!thread->stack)
        return -1;
    void *stack_top = (char *)thread->stack + STACK_SIZE + 16;
    stack_top = (void *)((uintptr_t)stack_top & ~0xF);

    g_info.thread = thread;
    g_info.func = start_routine;
    g_info.arg = arg;

    long flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | SIGCHLD;
    long ret = clone(start_thread, stack_top, flags, &g_info);
    if (ret == -1) {
        free(thread->stack);
        return -1;
    }
    thread->tid = (pid_t)ret;
    return 0;
}

int pthread_join(pthread_t *thread, void **retval)
{
    if (thread->detached)
        return -1;
    if (waitpid(thread->tid, NULL, 0) < 0)
        return -1;
    if (retval)
        *retval = thread->retval;
    free(thread->stack);
    return 0;
}

int pthread_detach(pthread_t *thread)
{
    thread->detached = 1;
    /* resources will be released automatically when the thread exits */
    return 0;
}

int pthread_mutex_init(pthread_mutex_t *mutex, void *attr)
{
    (void)attr;
    mutex->locked = 0;
    return 0;
}

int pthread_mutex_lock(pthread_mutex_t *mutex)
{
    while (__sync_lock_test_and_set(&mutex->locked, 1))
        ;
    return 0;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    __sync_lock_release(&mutex->locked);
    return 0;
}

int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
    (void)mutex;
    return 0;
}
