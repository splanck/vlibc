[← Back to index](index.md)

## Process Control

Process-related functionality resides in the **process** module. It provides
minimal wrappers for creating and managing processes, querying process IDs,
and installing signal handlers.  The companion `signal.h` header offers
`sigaction`, `sigprocmask` and helpers for manipulating signal sets:

```c
pid_t fork(void);
int execve(const char *pathname, char *const argv[], char *const envp[]);
int fexecve(int fd, char *const argv[], char *const envp[]);
int execvp(const char *file, char *const argv[]);
int execv(const char *path, char *const argv[]);
int execl(const char *path, const char *arg, ...);
int execlp(const char *file, const char *arg, ...);
int execle(const char *path, const char *arg, ...);
int posix_spawn(pid_t *pid, const char *path,
                const posix_spawn_file_actions_t *file_actions,
                const posix_spawnattr_t *attrp,
                char *const argv[], char *const envp[]);
int posix_spawnp(pid_t *pid, const char *file,
                 const posix_spawn_file_actions_t *file_actions,
                 const posix_spawnattr_t *attrp,
                 char *const argv[], char *const envp[]);
int posix_spawnattr_init(posix_spawnattr_t *attr);
int posix_spawnattr_setflags(posix_spawnattr_t *attr, short flags);
int posix_spawnattr_setsigmask(posix_spawnattr_t *attr, const sigset_t *mask);
int posix_spawn_file_actions_init(posix_spawn_file_actions_t *acts);
int posix_spawn_file_actions_addopen(posix_spawn_file_actions_t *acts, int fd,
                                     const char *path, int oflag, mode_t mode);
int posix_spawn_file_actions_adddup2(posix_spawn_file_actions_t *acts, int fd,
                                     int newfd);
int posix_spawn_file_actions_addclose(posix_spawn_file_actions_t *acts, int fd);
pid_t wait(int *status);
pid_t waitpid(pid_t pid, int *status, int options);
int kill(pid_t pid, int sig);
pid_t getpid(void);
pid_t getppid(void);
int setpgid(pid_t pid, pid_t pgid);
pid_t getpgid(pid_t pid);
int setpgrp(void);
pid_t getpgrp(void);
pid_t setsid(void);
pid_t getsid(pid_t pid);
sighandler_t signal(int signum, sighandler_t handler);
uid_t getuid(void);
uid_t geteuid(void);
gid_t getgid(void);
gid_t getegid(void);
int setuid(uid_t uid);
int seteuid(uid_t euid);
int setgid(gid_t gid);
int setegid(gid_t egid);
int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
int sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
int sigpending(sigset_t *set);
int sigsuspend(const sigset_t *mask);
int sigwait(const sigset_t *set, int *sig);
int sigwaitinfo(const sigset_t *set, siginfo_t *info);
int sigtimedwait(const sigset_t *set, siginfo_t *info,
                 const struct timespec *timeout);
int sigqueue(pid_t pid, int signo, const union sigval value);
int system(const char *command);
int daemon(int nochdir, int noclose);
int atexit(void (*fn)(void));
void abort(void);
void exit(int status);
```

These wrappers retrieve and manipulate process information. `getuid`,
`geteuid`, `getgid`, and `getegid` return the real and effective user and
group IDs. `setuid`, `seteuid`, `setgid`, and `setegid` modify them when
supported by the host.
Convenience wrappers `getpgrp()` and `setpgrp()` map to `getpgid(0)` and
`setpgid(0, 0)` for portability.
`sigqueue` delivers a queued signal with a small data payload when the
platform exposes `rt_sigqueueinfo` or falls back to the native implementation.
`sigaltstack` configures an alternate stack for signal handlers using the
`sigaltstack` system call when available or delegating to the host on BSD.

`posix_spawn` accepts an attribute object controlling the signal mask and
process group of the new process. File actions can be supplied to open, close
or duplicate file descriptors before executing the program.  On the BSD
family the implementation delegates to the native spawn facilities provided
by the kernel.  When available, `posix_spawn` or a combination of
`pdfork` and `fdwalk` is used to create the child efficiently.  Systems
lacking these features fall back to a portable `fork`/`exec` sequence that
communicates early errors over a pipe.

### Example

```c
/* Spawn a child that prints a message and wait for it to finish. */
pid_t pid;
char *args[] = {"/bin/echo", "hello", NULL};
posix_spawn(&pid, "/bin/echo", NULL, NULL, args, environ);
waitpid(pid, NULL, 0);

/* Spawn echo with stdout redirected and SIGUSR1 blocked. */
posix_spawn_file_actions_t fa;
posix_spawn_file_actions_init(&fa);
posix_spawn_file_actions_addopen(&fa, 1, "/tmp/out.txt",
                                 O_WRONLY|O_CREAT|O_TRUNC, 0600);

posix_spawnattr_t at;
posix_spawnattr_init(&at);
sigset_t m;
sigemptyset(&m);
sigaddset(&m, SIGUSR1);
posix_spawnattr_setflags(&at, POSIX_SPAWN_SETSIGMASK);
posix_spawnattr_setsigmask(&at, &m);
posix_spawn(&pid, "/bin/echo", &fa, &at, args, environ);

/* Install a handler and send the process an interrupt. */
void on_int(int signo) { (void)signo; }
signal(SIGINT, on_int);
kill(getpid(), SIGINT);

/* Detach and run in the background. */
if (daemon(0, 0) < 0)
    perror("daemon");
```

`execvp` performs the same operation as `execve` but searches the directories in the `PATH` environment variable when the program name does not contain a slash.
`execv`, `execl`, `execlp` and `execle` are thin wrappers that build an
argument array on behalf of the caller. `execl` and `execlp` accept a
variable list of arguments terminated by `NULL`. `execle` is similar but
takes a custom environment pointer after the final `NULL` argument.
`fexecve` is provided for executing a program referenced by an open file descriptor. When the `fexecve` system call is unavailable on BSD systems, vlibc falls back to invoking `/dev/fd/<fd>` through `execve`.

### Wait Status Helpers

Macros in `<sys/wait.h>` decode the integer status returned by `wait()`
and `waitpid()`. Use `WIFEXITED(status)` to detect normal termination
and `WEXITSTATUS(status)` to read the child's exit code. When a signal
causes termination, `WIFSIGNALED(status)` becomes true and
`WTERMSIG(status)` yields the signal number. `WIFSTOPPED(status)` tests
if the child was stopped by a signal.

The convenience `system()` call executes a shell command by forking and
invoking `/bin/sh -c command`. It returns the raw status from `waitpid`
and is intended only for simple helper tasks.
`popen` and `pclose` provide a lightweight way to run a command with a
pipe connected to the child. Use mode `"r"` to read its output or `"w"`
to send data to its stdin.
`abort()` sends `SIGABRT` to the current process and does not invoke
`atexit` handlers.

`exit()` terminates the process after running any handlers registered with `atexit()`. The handlers execute in reverse registration order. `_exit()` bypasses them.
The design favors straightforward semantics over comprehensive POSIX
conformance.

### Quick-Exit Handlers

Handlers registered via `at_quick_exit` run when `quick_exit` is called. They
execute in reverse order of registration and are intended for simple cleanup
prior to terminating the process immediately. After running the handlers
`quick_exit` invokes `_exit` without flushing stdio buffers or calling regular
`atexit` handlers.

## Error Reporting

vlibc provides minimal helpers to report errors:

```c
const char *strerror(int errnum);
int strerror_r(int errnum, char *buf, size_t buflen);
void perror(const char *s);
const char *strsignal(int signum);
```

Additional helpers mirror the traditional BSD `err.h` API:

```c
void warn(const char *fmt, ...);
void warnx(const char *fmt, ...);
void err(int status, const char *fmt, ...) __attribute__((noreturn));
void errx(int status, const char *fmt, ...) __attribute__((noreturn));
```

`warn` prints the formatted message followed by the text for the current
`errno`. The `err` variants behave similarly but terminate the process with
the provided exit status. The `warnx`/`errx` forms omit the `errno`
description.


`strerror()` returns a string describing `errnum` or "Unknown error" for
codes it does not recognize. `strerror_r()` is a thread-safe variant that
writes the message into `buf`. `perror()` writes a message to `stderr`
combining the optional prefix with the text for the current `errno`.
`strsignal()` maps a signal number to a short descriptive string or
"Unknown signal" when the name is not recognized.

## Errno Access

The `errno` variable is thread‑local and stores the last failure code.
The helper `__errno_location()` returns a pointer to this value so that
macros can reference it directly.

```c
extern __thread int errno;
int *__errno_location(void);
```

## Threading

Basic thread support is delegated to the host's `pthread` library.
Only a handful of functions are provided:

```c
int pthread_create(pthread_t *thread, const void *attr,
                   void *(*start)(void *), void *arg);
int pthread_join(pthread_t thread, void **retval);
int pthread_detach(pthread_t thread);
pthread_t pthread_self(void);
int pthread_equal(pthread_t a, pthread_t b);
void pthread_exit(void *retval);
int pthread_cancel(pthread_t thread);

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

typedef struct { atomic_flag locked; } pthread_spinlock_t;
int pthread_spin_init(pthread_spinlock_t *lock, int pshared);
int pthread_spin_lock(pthread_spinlock_t *lock);
int pthread_spin_trylock(pthread_spinlock_t *lock);
int pthread_spin_unlock(pthread_spinlock_t *lock);
int pthread_spin_destroy(pthread_spinlock_t *lock);

typedef struct { atomic_int count; } sem_t;
int sem_init(sem_t *sem, int pshared, unsigned value);
int sem_destroy(sem_t *sem);
int sem_wait(sem_t *sem);
int sem_trywait(sem_t *sem);
int sem_post(sem_t *sem);
sem_t *sem_open(const char *name, int oflag, ...);
int sem_close(sem_t *sem);
int sem_unlink(const char *name);
int sem_getvalue(sem_t *sem, int *value);
int sem_timedwait(sem_t *sem, const struct timespec *abstime);

int pthread_barrier_init(pthread_barrier_t *barrier, void *attr,
                         unsigned count);
int pthread_barrier_wait(pthread_barrier_t *barrier);
int pthread_barrier_destroy(pthread_barrier_t *barrier);
```

Threads share the process address space and use a simple spinlock-based
mutex for synchronization.

`pthread_self()` returns the identifier of the calling thread while
`pthread_equal()` compares two thread IDs for equality.

`pthread_create()` spawns a new thread running the `start` routine with the
given argument. The thread identifier is written to `thread` and can later be
passed to `pthread_join()` or `pthread_detach()`.

`pthread_join()` waits for a joinable thread to finish and retrieves the value
returned by the start routine. It should only be called once per thread.

`pthread_detach()` marks a thread so that its resources are reclaimed
automatically when it terminates. Detached threads cannot be joined.

`pthread_exit()` terminates the calling thread immediately and supplies the
given value to any `pthread_join()` waiting on it. Another thread can request
cancellation with `pthread_cancel()`. When the target reaches a cancellation
point it exits and the join result is `PTHREAD_CANCELED`.

Mutex routines provide minimal mutual exclusion. `pthread_mutex_init()`
initializes a mutex, `pthread_mutex_lock()` acquires it,
`pthread_mutex_trylock()` attempts to lock without blocking and returns
`EBUSY` if the mutex is already held, and `pthread_mutex_unlock()`
releases it.  Destroying a locked mutex with `pthread_mutex_destroy()` is
undefined.

Mutex attributes currently track only the mutex type. Use
`pthread_mutexattr_settype()` with `PTHREAD_MUTEX_NORMAL` (the default)
or `PTHREAD_MUTEX_RECURSIVE` and pass the attribute to
`pthread_mutex_init()`.

Spin locks provide even lighter mutual exclusion. `pthread_spin_lock()`
and `pthread_spin_trylock()` busy-wait until the lock becomes
available and should only guard short sections of code. They do not
support process-shared locks or advanced features like priority
inheritance.

Condition variables provide simple waiting semantics. A thread calls
`pthread_cond_wait()` with a locked mutex and blocks until another thread
signals the condition. `pthread_cond_timedwait()` behaves the same but
returns `ETIMEDOUT` if the absolute time specified by `abstime` is
reached. `pthread_cond_signal()` wakes a single waiter while
`pthread_cond_broadcast()` wakes all waiters.

Read-write locks allow multiple threads to hold the lock in read mode or
a single writer to hold it exclusively. They are lightweight wrappers
around atomic counters and follow the same initialization and destruction
pattern as mutexes.

Semaphores provide a simple counting mechanism for coordinating threads. They
use an atomic counter and block with `nanosleep` when no resources are
available.
Named semaphores can be opened with `sem_open`. On BSD systems the native
interfaces are used while other platforms allocate an in-process semaphore and
ignore the name. Such fallbacks are not shared between processes and
`sem_unlink` simply succeeds.

Barriers synchronize a fixed number of threads at predetermined points. After
calling `pthread_barrier_init()` with the participant count, each thread invokes
`pthread_barrier_wait()` to pause until all threads reach the barrier. The last
thread to arrive receives `PTHREAD_BARRIER_SERIAL_THREAD` and releases the rest.

Thread-local storage is available through key objects:

```c
int pthread_key_create(pthread_key_t *key, void (*destructor)(void *));
int pthread_key_delete(pthread_key_t key);
int pthread_setspecific(pthread_key_t key, const void *value);
void *pthread_getspecific(pthread_key_t key);

int pthread_once(pthread_once_t *control, void (*init)(void));
```

`pthread_once` guarantees that `init` runs just once even if multiple
threads call it concurrently.

### Example

```c
#include "pthread.h"
#include <stdio.h>

static pthread_mutex_t lock;

static void *worker(void *arg) {
    int *val = arg;
    for (int i = 0; i < 1000; ++i) {
        pthread_mutex_lock(&lock);
        (*val)++;
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

int main(void) {
    pthread_t t1, t2;
    int counter = 0;

    pthread_mutex_init(&lock, NULL);
    pthread_create(&t1, NULL, worker, &counter);
    pthread_create(&t2, NULL, worker, &counter);

    pthread_join(&t1, NULL);
    pthread_join(&t2, NULL);

    printf("final: %d\n", counter);
    pthread_mutex_destroy(&lock);
    return 0;
}
```

### Barrier Example

```c
#include "pthread.h"
#include <stdio.h>

static pthread_barrier_t bar;

static void *task(void *arg)
{
    printf("phase 1\n");
    pthread_barrier_wait(&bar);
    printf("phase 2\n");
    return NULL;
}

int main(void)
{
    pthread_t t1, t2;
    pthread_barrier_init(&bar, NULL, 3);

    pthread_create(&t1, NULL, task, NULL);
    pthread_create(&t2, NULL, task, NULL);

    pthread_barrier_wait(&bar);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_barrier_destroy(&bar);
    return 0;
}
```

## Dynamic Loading

The `dlfcn` module implements a minimal ELF loader. On 64-bit x86
systems it can resolve `R_X86_64_RELATIVE`, `R_X86_64_64`,
`R_X86_64_GLOB_DAT`, and `R_X86_64_JUMP_SLOT` relocations. Other
architectures are currently unsupported and attempting to load a
library will fail with an error. Use `dlopen`, `dlsym`, `dlclose` and
`dladdr` to load code and query symbol information at runtime.

```c
Dl_info info;
void *handle = dlopen("plugin.so", RTLD_NOW);
void *sym = dlsym(handle, "plugin_value");
if (dladdr(sym, &info))
    printf("%s from %s\n", info.dli_sname, info.dli_fname);
```

## Environment Variables

The environment module exposes a global pointer `environ` storing the
process's `name=value` pairs. Programs with a custom entry point should
call `env_init(envp)` before using `getenv`, `setenv`, `putenv`, or
`unsetenv`. All variables can be removed at once with `clearenv()`.

```c
extern char **environ;

int main(int argc, char **argv, char **envp) {
    env_init(envp);
    setenv("FOO", "BAR", 1);
    putenv("BAR=BAZ");
    const char *v = getenv("FOO");
    unsetenv("FOO");
    clearenv();
    return 0;
}
```

## System Information

Use `uname` from `sys/utsname.h` to query the running operating system:

```c
struct utsname u;
if (uname(&u) == 0) {
    printf("%s %s\n", u.sysname, u.release);
}
```

The `confstr()` function retrieves string-valued system parameters such as
`_CS_PATH`. When the query is unsupported it returns `0` and sets `errno` to
`EINVAL`.

```c
char path[256];
size_t n = confstr(_CS_PATH, path, sizeof(path));
if (n > 0 && n < sizeof(path))
    printf("search path: %s\n", path);
```

