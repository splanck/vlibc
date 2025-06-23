#ifndef PROCESS_H
#define PROCESS_H

#define _POSIX_C_SOURCE 200809L
#include <sys/types.h>
#include <signal.h>

pid_t fork(void);
int execve(const char *pathname, char *const argv[], char *const envp[]);
int execvp(const char *file, char *const argv[]);
int execv(const char *path, char *const argv[]);
int execl(const char *path, const char *arg, ...);
int execlp(const char *file, const char *arg, ...);
int execle(const char *path, const char *arg, ...);
pid_t waitpid(pid_t pid, int *status, int options);
int kill(pid_t pid, int sig);
pid_t getpid(void);
pid_t getppid(void);
int setpgid(pid_t pid, pid_t pgid);
pid_t getpgid(pid_t pid);
pid_t setsid(void);
pid_t getsid(pid_t pid);

void _exit(int status);
void exit(int status);

typedef void (*sighandler_t)(int);
sighandler_t signal(int signum, sighandler_t handler);

typedef struct { int __dummy; } posix_spawnattr_t;
typedef struct { int __dummy; } posix_spawn_file_actions_t;
int posix_spawn(pid_t *pid, const char *path,
                const posix_spawn_file_actions_t *file_actions,
                const posix_spawnattr_t *attrp,
                char *const argv[], char *const envp[]);
int posix_spawnp(pid_t *pid, const char *file,
                 const posix_spawn_file_actions_t *file_actions,
                 const posix_spawnattr_t *attrp,
                 char *const argv[], char *const envp[]);

#endif /* PROCESS_H */
