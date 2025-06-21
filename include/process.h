#ifndef PROCESS_H
#define PROCESS_H

#define _POSIX_C_SOURCE 200809L
#include <sys/types.h>
#include <signal.h>

pid_t fork(void);
int execve(const char *pathname, char *const argv[], char *const envp[]);
pid_t waitpid(pid_t pid, int *status, int options);
int kill(pid_t pid, int sig);

typedef void (*sighandler_t)(int);
sighandler_t signal(int signum, sighandler_t handler);

#endif /* PROCESS_H */
