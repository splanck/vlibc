#ifndef SYS_STAT_H
#define SYS_STAT_H

#include <sys/types.h>

#if defined(__has_include)
#  if __has_include("/usr/include/x86_64-linux-gnu/sys/stat.h")
#    include "/usr/include/x86_64-linux-gnu/sys/stat.h"
#  elif __has_include("/usr/include/sys/stat.h")
#    include "/usr/include/sys/stat.h"
#  endif
#endif

struct stat;

int stat(const char *path, struct stat *buf);
int fstat(int fd, struct stat *buf);
int fstatat(int dirfd, const char *path, struct stat *buf, int flags);
int lstat(const char *path, struct stat *buf);
int mkfifo(const char *path, mode_t mode);
int mkfifoat(int dirfd, const char *path, mode_t mode);

#endif /* SYS_STAT_H */
