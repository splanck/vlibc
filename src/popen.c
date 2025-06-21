#include "stdio.h"
#include "process.h"
#include "io.h"
#include "memory.h"
#include "string.h"

struct popen_file {
    FILE file;
    pid_t pid;
};

FILE *popen(const char *command, const char *mode)
{
    if (!command || !mode)
        return NULL;

    int read_mode = (mode[0] == 'r');
    int write_mode = (mode[0] == 'w');
    if (!read_mode && !write_mode)
        return NULL;

    int pipefd[2];
    if (pipe(pipefd) < 0)
        return NULL;

    pid_t pid = fork();
    if (pid < 0) {
        close(pipefd[0]);
        close(pipefd[1]);
        return NULL;
    }

    if (pid == 0) {
        if (read_mode) {
            dup2(pipefd[1], 1);
            close(pipefd[0]);
            close(pipefd[1]);
        } else {
            dup2(pipefd[0], 0);
            close(pipefd[1]);
            close(pipefd[0]);
        }
        char *argv[] = {"/bin/sh", "-c", (char *)command, NULL};
        extern char **environ;
        execve("/bin/sh", argv, environ);
        _exit(127);
    }

    struct popen_file *pf = malloc(sizeof(struct popen_file));
    if (!pf) {
        close(pipefd[0]);
        close(pipefd[1]);
        return NULL;
    }
    pf->pid = pid;
    if (read_mode) {
        pf->file.fd = pipefd[0];
        close(pipefd[1]);
    } else {
        pf->file.fd = pipefd[1];
        close(pipefd[0]);
    }
    return &pf->file;
}

int pclose(FILE *stream)
{
    if (!stream)
        return -1;
    struct popen_file *pf = (struct popen_file *)stream;
    pid_t pid = pf->pid;
    close(stream->fd);
    free(pf);
    int status = 0;
    if (waitpid(pid, &status, 0) < 0)
        return -1;
    return status;
}

