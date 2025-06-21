#ifndef IO_H
#define IO_H

#include <sys/types.h>

int open(const char *path, int flags, ...);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
int close(int fd);

#endif /* IO_H */
