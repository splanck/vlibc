/*
 * BSD 2-Clause License
 *
 * Purpose: Declarations for POSIX I/O and process helpers.
 */
#ifndef UNISTD_H
#define UNISTD_H

#if defined(__has_include)
#  if __has_include_next(<unistd.h>)
#    include_next <unistd.h>
#  endif
#endif

#include <sys/types.h>
#include "io.h"
#include "process.h"
#include "env.h"

int isatty(int fd);
int daemon(int nochdir, int noclose);
int chroot(const char *path);

uid_t getuid(void);
uid_t geteuid(void);
gid_t getgid(void);
gid_t getegid(void);
int setuid(uid_t uid);
int seteuid(uid_t euid);
int setgid(gid_t gid);
int setegid(gid_t egid);

long sysconf(int name);
int getpagesize(void);

size_t confstr(int name, char *buf, size_t len);

long pathconf(const char *path, int name);
long fpathconf(int fd, int name);

char *getlogin(void);
char *getpass(const char *prompt);

#ifndef STDIN_FILENO
#define STDIN_FILENO 0
#endif
#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif
#ifndef STDERR_FILENO
#define STDERR_FILENO 2
#endif

#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

#endif /* UNISTD_H */
