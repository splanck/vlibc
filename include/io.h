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
int dup3(int oldfd, int newfd, int flags);
int pipe(int pipefd[2]);
int pipe2(int pipefd[2], int flags);
int ftruncate(int fd, off_t length);
int truncate(const char *path, off_t length);
int unlink(const char *pathname);
int rename(const char *oldpath, const char *newpath);
int symlink(const char *target, const char *linkpath);
int mkdir(const char *pathname, mode_t mode);
int rmdir(const char *pathname);
int chdir(const char *path);

#endif /* IO_H */
