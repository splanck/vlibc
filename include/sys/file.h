#ifndef SYS_FILE_H
#define SYS_FILE_H

#include <sys/types.h>

int chmod(const char *path, mode_t mode);
int chown(const char *path, uid_t owner, gid_t group);
mode_t umask(mode_t mask);

#endif /* SYS_FILE_H */

#include_next <sys/file.h>
