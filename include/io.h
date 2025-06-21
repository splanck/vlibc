#ifndef IO_H
#define IO_H

#include <sys/types.h>

int open(const char *path, int flags, ...);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
int close(int fd);
off_t lseek(int fd, off_t offset, int whence);
int dup(int oldfd);
int dup2(int oldfd, int newfd);
int pipe(int pipefd[2]);
int unlink(const char *pathname);
int rename(const char *oldpath, const char *newpath);

#endif /* IO_H */
