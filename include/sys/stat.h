#ifndef SYS_STAT_H
#define SYS_STAT_H

#include <sys/types.h>

struct stat;

int stat(const char *path, struct stat *buf);
int fstat(int fd, struct stat *buf);
int lstat(const char *path, struct stat *buf);

#endif /* SYS_STAT_H */

#include_next <sys/stat.h>
