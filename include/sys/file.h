#ifndef SYS_FILE_H
#define SYS_FILE_H

#include <sys/types.h>

#if defined(__has_include)
#  if __has_include("/usr/include/x86_64-linux-gnu/sys/file.h")
#    include "/usr/include/x86_64-linux-gnu/sys/file.h"
#  elif __has_include("/usr/include/sys/file.h")
#    include "/usr/include/sys/file.h"
#  endif
#endif

int chmod(const char *path, mode_t mode);
int chown(const char *path, uid_t owner, gid_t group);
mode_t umask(mode_t mask);

#endif /* SYS_FILE_H */
