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
#include <stdarg.h>
#include <fcntl.h>
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

int execv(const char *path, char *const argv[])
{
    return execve(path, argv, environ);
}

static int vlibc_build_argv(const char *arg, va_list ap, char ***out)
{
    va_list ap_copy;
    va_copy(ap_copy, ap);
    size_t count = 1;
    const char *p = arg;
    while (p) {
        p = va_arg(ap_copy, char *);
        if (p)
            count++;
    }
    va_end(ap_copy);

    char **argv = malloc(sizeof(char *) * (count + 1));
    if (!argv)
        return -1;

    argv[0] = (char *)arg;
    for (size_t i = 1; i <= count; i++) {
        argv[i] = (char *)va_arg(ap, char *);
        if (!argv[i]) {
            argv[count] = NULL;
            break;
        }
    }

    *out = argv;
    return 0;
}

int execl(const char *path, const char *arg, ...)
{
    va_list ap;
    va_start(ap, arg);
    char **argv;
    if (vlibc_build_argv(arg, ap, &argv) < 0) {
        va_end(ap);
        return -1;
    }
    va_end(ap);
    int r = execve(path, argv, environ);
    free(argv);
    return r;
}

int execlp(const char *file, const char *arg, ...)
{
    va_list ap;
    va_start(ap, arg);
    char **argv;
    if (vlibc_build_argv(arg, ap, &argv) < 0) {
        va_end(ap);
        return -1;
    }
    va_end(ap);
    int r = execvp(file, argv);
    free(argv);
    return r;
}

int execle(const char *path, const char *arg, ...)
{
    va_list ap;
    va_start(ap, arg);
    va_list ap_copy;
    va_copy(ap_copy, ap);
    size_t count = 1;
    const char *p = arg;
    while ((p = va_arg(ap_copy, char *)))
        count++;
    char **envp = va_arg(ap_copy, char **);
    va_end(ap_copy);

    char **argv = malloc(sizeof(char *) * (count + 1));
    if (!argv) {
        va_end(ap);
        return -1;
    }

    argv[0] = (char *)arg;
    for (size_t i = 1; i <= count; i++) {
        argv[i] = (char *)va_arg(ap, char *);
        if (!argv[i]) {
            argv[count] = NULL;
            break;
        }
    }
    va_end(ap);

    int r = execve(path, argv, envp);
    free(argv);
    return r;
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

pid_t wait(int *status)
{
    return waitpid(-1, status, 0);
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

int setpgid(pid_t pid, pid_t pgid)
{
#ifdef SYS_setpgid
    long ret = vlibc_syscall(SYS_setpgid, pid, pgid, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
#else
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    extern int host_setpgid(pid_t, pid_t) __asm__("setpgid");
    return host_setpgid(pid, pgid);
#else
    errno = ENOSYS;
    return -1;
#endif
#endif
}

pid_t getpgid(pid_t pid)
{
#ifdef SYS_getpgid
    long ret = vlibc_syscall(SYS_getpgid, pid, 0, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (pid_t)ret;
#else
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    extern pid_t host_getpgid(pid_t) __asm__("getpgid");
    return host_getpgid(pid);
#else
    errno = ENOSYS;
    return -1;
#endif
#endif
}

pid_t setsid(void)
{
#ifdef SYS_setsid
    long ret = vlibc_syscall(SYS_setsid, 0, 0, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (pid_t)ret;
#else
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    extern pid_t host_setsid(void) __asm__("setsid");
    return host_setsid();
#else
    errno = ENOSYS;
    return -1;
#endif
#endif
}

pid_t getsid(pid_t pid)
{
#ifdef SYS_getsid
    long ret = vlibc_syscall(SYS_getsid, pid, 0, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (pid_t)ret;
#else
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    extern pid_t host_getsid(pid_t) __asm__("getsid");
    return host_getsid(pid);
#else
    errno = ENOSYS;
    return -1;
#endif
#endif
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

struct posix_spawn_file_action {
    int type;
    int fd;
    int newfd;
    char *path;
    int oflag;
    mode_t mode;
};

#define SPAWN_ACTION_OPEN  1
#define SPAWN_ACTION_CLOSE 2
#define SPAWN_ACTION_DUP2  3

int posix_spawn_file_actions_init(posix_spawn_file_actions_t *acts)
{
    if (!acts)
        return EINVAL;
    acts->actions = NULL;
    acts->count = 0;
    return 0;
}

int posix_spawn_file_actions_destroy(posix_spawn_file_actions_t *acts)
{
    if (!acts)
        return EINVAL;
    for (size_t i = 0; i < acts->count; i++)
        if (acts->actions[i].type == SPAWN_ACTION_OPEN)
            free(acts->actions[i].path);
    free(acts->actions);
    acts->actions = NULL;
    acts->count = 0;
    return 0;
}

static int file_actions_add(posix_spawn_file_actions_t *acts,
                            struct posix_spawn_file_action **out)
{
    size_t n = acts->count + 1;
    struct posix_spawn_file_action *tmp =
        realloc(acts->actions, n * sizeof(*tmp));
    if (!tmp)
        return ENOMEM;
    acts->actions = tmp;
    *out = &acts->actions[acts->count];
    acts->count = n;
    return 0;
}

int posix_spawn_file_actions_addopen(posix_spawn_file_actions_t *acts, int fd,
                                     const char *path, int oflag, mode_t mode)
{
    if (!acts || !path)
        return EINVAL;
    struct posix_spawn_file_action *a;
    int r = file_actions_add(acts, &a);
    if (r)
        return r;
    a->type = SPAWN_ACTION_OPEN;
    a->fd = fd;
    a->oflag = oflag;
    a->mode = mode;
    a->path = strdup(path);
    if (!a->path)
        return ENOMEM;
    return 0;
}

int posix_spawn_file_actions_adddup2(posix_spawn_file_actions_t *acts, int fd,
                                     int newfd)
{
    if (!acts)
        return EINVAL;
    struct posix_spawn_file_action *a;
    int r = file_actions_add(acts, &a);
    if (r)
        return r;
    a->type = SPAWN_ACTION_DUP2;
    a->fd = fd;
    a->newfd = newfd;
    a->path = NULL;
    return 0;
}

int posix_spawn_file_actions_addclose(posix_spawn_file_actions_t *acts, int fd)
{
    if (!acts)
        return EINVAL;
    struct posix_spawn_file_action *a;
    int r = file_actions_add(acts, &a);
    if (r)
        return r;
    a->type = SPAWN_ACTION_CLOSE;
    a->fd = fd;
    a->path = NULL;
    return 0;
}

int posix_spawnattr_init(posix_spawnattr_t *attr)
{
    if (!attr)
        return EINVAL;
    attr->flags = 0;
    sigemptyset(&attr->sigmask);
    sigemptyset(&attr->sigdefault);
    attr->pgroup = 0;
    return 0;
}

int posix_spawnattr_destroy(posix_spawnattr_t *attr)
{
    (void)attr;
    return 0;
}

int posix_spawnattr_setflags(posix_spawnattr_t *attr, short flags)
{
    if (!attr)
        return EINVAL;
    attr->flags = flags;
    return 0;
}

int posix_spawnattr_getflags(const posix_spawnattr_t *attr, short *flags)
{
    if (!attr || !flags)
        return EINVAL;
    *flags = attr->flags;
    return 0;
}

int posix_spawnattr_setsigmask(posix_spawnattr_t *attr, const sigset_t *mask)
{
    if (!attr || !mask)
        return EINVAL;
    attr->sigmask = *mask;
    return 0;
}

int posix_spawnattr_getsigmask(const posix_spawnattr_t *attr, sigset_t *mask)
{
    if (!attr || !mask)
        return EINVAL;
    *mask = attr->sigmask;
    return 0;
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
    int errpipe[2];
    if (pipe(errpipe) < 0)
        return errno;
    fcntl(errpipe[0], F_SETFD, FD_CLOEXEC);
    fcntl(errpipe[1], F_SETFD, FD_CLOEXEC);

    pid_t cpid = fork();
    if (cpid < 0)
        {
            int err = errno;
            close(errpipe[0]);
            close(errpipe[1]);
            return err;
        }
    if (cpid == 0) {
        close(errpipe[0]);
        int err;

        if (attrp) {
            if (attrp->flags & POSIX_SPAWN_SETPGROUP)
                setpgid(0, attrp->pgroup);
            if (attrp->flags & POSIX_SPAWN_SETSIGMASK)
                sigprocmask(SIG_SETMASK, &attrp->sigmask, NULL);
            if (attrp->flags & POSIX_SPAWN_SETSIGDEF)
                for (int s = 1; s < 64; s++)
                    if (sigismember(&attrp->sigdefault, s))
                        signal(s, SIG_DFL);
        }

        if (file_actions) {
            for (size_t i = 0; i < file_actions->count; i++) {
                struct posix_spawn_file_action *a = &file_actions->actions[i];
                switch (a->type) {
                case SPAWN_ACTION_OPEN: {
                    int fd = open(a->path, a->oflag, a->mode);
                    if (fd < 0) { err = errno; write(errpipe[1], &err, sizeof(err)); _exit(127); }
                    if (fd != a->fd) {
                        if (dup2(fd, a->fd) < 0) { err = errno; write(errpipe[1], &err, sizeof(err)); _exit(127); }
                        close(fd);
                    }
                    break; }
                case SPAWN_ACTION_CLOSE:
                    if (close(a->fd) < 0) { err = errno; write(errpipe[1], &err, sizeof(err)); _exit(127); }
                    break;
                case SPAWN_ACTION_DUP2:
                    if (dup2(a->fd, a->newfd) < 0) { err = errno; write(errpipe[1], &err, sizeof(err)); _exit(127); }
                    break;
                }
            }
        }

        execve(path, argv, envp ? envp : environ);
        err = errno;
        write(errpipe[1], &err, sizeof(err));
        _exit(127);
    }
    close(errpipe[1]);
    int child_err = 0;
    ssize_t n = read(errpipe[0], &child_err, sizeof(child_err));
    close(errpipe[0]);
    if (n > 0) {
        waitpid(cpid, NULL, 0);
        return child_err;
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
