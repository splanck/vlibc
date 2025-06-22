#include "process.h"
#include "stdlib.h"
#include "string.h"
#include "vlibc.h"

/* Simple implementation of system(3) using fork/execve/waitpid */
int system(const char *command)
{
    if (!command)
        return 1;

    pid_t pid = fork();
    if (pid < 0)
        return -1;
    if (pid == 0) {
        const char *shell = vlibc_default_shell();
        char *argv[] = {(char *)shell, "-c", (char *)command, NULL};
        extern char **environ;
        execve(shell, argv, environ);
        _exit(127);
    }

    int status = 0;
    if (waitpid(pid, &status, 0) < 0)
        return -1;
    return status;
}

