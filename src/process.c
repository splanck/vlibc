#include "process.h"
#include "errno.h"
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <signal.h>
#include "string.h"
#include "syscall.h"
#include "env.h"
#include "memory.h"
extern long syscall(long number, ...);

/* from atexit.c */
extern void __run_atexit(void);


pid_t fork(void)
{
#ifdef SYS_fork
    long ret = vlibc_syscall(SYS_fork, 0, 0, 0, 0, 0, 0);
#else
    long ret = vlibc_syscall(SYS_clone, SIGCHLD, 0, 0, 0, 0, 0);
#endif
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (pid_t)ret;
}

int execve(const char *pathname, char *const argv[], char *const envp[])
{
    long ret = vlibc_syscall(SYS_execve, (long)pathname, (long)argv, (long)envp, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
}

int execvp(const char *file, char *const argv[])
{
    if (!file || !argv)
        return -1;

    if (strchr(file, '/'))
        return execve(file, argv, environ);

    const char *path = getenv("PATH");
    if (!path)
        return execve(file, argv, environ);

    size_t flen = strlen(file);
    const char *p = path;
    int last_errno = ENOENT;

    while (1) {
        const char *end = strchr(p, ':');
        size_t len = end ? (size_t)(end - p) : strlen(p);

        size_t tlen = len ? len : 1; /* allow empty meaning current dir */
        char *buf = malloc(tlen + 1 + flen + 1);
        if (!buf)
            return -1;

        if (len) {
            memcpy(buf, p, len);
            buf[len] = '/';
            memcpy(buf + len + 1, file, flen);
            buf[len + 1 + flen] = '\0';
        } else {
            memcpy(buf, file, flen);
            buf[flen] = '\0';
        }

        execve(buf, argv, environ);
        last_errno = errno;
        free(buf);

        if (!end)
            break;
        p = end + 1;
    }

    errno = last_errno;
    return -1;
}

pid_t waitpid(pid_t pid, int *status, int options)
{
#ifdef SYS_waitpid
    long ret = vlibc_syscall(SYS_waitpid, pid, (long)status, options, 0, 0, 0);
#else
    long ret = vlibc_syscall(SYS_wait4, pid, (long)status, options, 0, 0, 0);
#endif
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (pid_t)ret;
}

int kill(pid_t pid, int sig)
{
    long ret = vlibc_syscall(SYS_kill, pid, sig, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
}

pid_t getpid(void)
{
    long ret = vlibc_syscall(SYS_getpid, 0, 0, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (pid_t)ret;
}

pid_t getppid(void)
{
    long ret = vlibc_syscall(SYS_getppid, 0, 0, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (pid_t)ret;
}


void _exit(int status)
{
    vlibc_syscall(SYS_exit, status, 0, 0, 0, 0, 0);
    for (;;) {}
}

void exit(int status)
{
    /* No stdio buffering yet, so nothing to flush. */
    __run_atexit();
    _exit(status);
}

sighandler_t signal(int signum, sighandler_t handler)
{
#ifdef SYS_rt_sigaction
    struct sigaction act, old;
    vmemset(&act, 0, sizeof(act));
    act.sa_handler = handler;
    long ret = vlibc_syscall(SYS_rt_sigaction, signum, (long)&act, (long)&old, sizeof(sigset_t), 0, 0);
    if (ret < 0) {
        errno = -ret;
        return SIG_ERR;
    }
    return old.sa_handler;
#else
    long ret = vlibc_syscall(SYS_signal, signum, (long)handler, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return SIG_ERR;
    }
    return (sighandler_t)ret;
#endif
}

static pid_t vlibc_vfork(void)
{
#ifdef SYS_vfork
    long ret = vlibc_syscall(SYS_vfork, 0, 0, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (pid_t)ret;
#else
    return fork();
#endif
}

int posix_spawn(pid_t *pid, const char *path,
                const posix_spawn_file_actions_t *file_actions,
                const posix_spawnattr_t *attrp,
                char *const argv[], char *const envp[])
{
    (void)file_actions;
    (void)attrp;
    pid_t cpid = vlibc_vfork();
    if (cpid < 0)
        return errno;
    if (cpid == 0) {
        execve(path, argv, envp ? envp : environ);
        _exit(127);
    }
    if (pid)
        *pid = cpid;
    return 0;
}

int posix_spawnp(pid_t *pid, const char *file,
                 const posix_spawn_file_actions_t *file_actions,
                 const posix_spawnattr_t *attrp,
                 char *const argv[], char *const envp[])
{
    if (!file)
        return EINVAL;

    if (strchr(file, '/'))
        return posix_spawn(pid, file, file_actions, attrp, argv, envp);

    const char *path = getenv("PATH");
    if (!path)
        return posix_spawn(pid, file, file_actions, attrp, argv, envp);

    size_t flen = strlen(file);
    const char *p = path;
    int last_error = ENOENT;

    while (1) {
        const char *end = strchr(p, ':');
        size_t len = end ? (size_t)(end - p) : strlen(p);

        size_t tlen = len ? len : 1;
        char *buf = malloc(tlen + 1 + flen + 1);
        if (!buf)
            return ENOMEM;

        if (len) {
            memcpy(buf, p, len);
            buf[len] = '/';
            memcpy(buf + len + 1, file, flen);
            buf[len + 1 + flen] = '\0';
        } else {
            memcpy(buf, file, flen);
            buf[flen] = '\0';
        }

        int r = posix_spawn(pid, buf, file_actions, attrp, argv, envp);
        if (r == 0) {
            free(buf);
            return 0;
        }
        last_error = r;
        free(buf);

        if (!end)
            break;
        p = end + 1;
    }

    return last_error;
}
