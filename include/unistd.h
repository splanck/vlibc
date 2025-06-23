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

uid_t getuid(void);
uid_t geteuid(void);
gid_t getgid(void);
gid_t getegid(void);

/* sysconf keys */
#ifndef _SC_ARG_MAX
#define _SC_ARG_MAX 1
#endif
#ifndef _SC_PAGESIZE
#define _SC_PAGESIZE 2
#endif
#ifndef _SC_PAGE_SIZE
#define _SC_PAGE_SIZE _SC_PAGESIZE
#endif
#ifndef _SC_OPEN_MAX
#define _SC_OPEN_MAX 3
#endif
#ifndef _SC_NPROCESSORS_ONLN
#define _SC_NPROCESSORS_ONLN 4
#endif
#ifndef _SC_CLK_TCK
#define _SC_CLK_TCK 5
#endif

long sysconf(int name);

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
