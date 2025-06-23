#ifndef UNISTD_H
#define UNISTD_H

#if defined(__has_include)
#  if __has_include_next(<unistd.h>)
#    include_next <unistd.h>
#  endif
#endif

int isatty(int fd);

#include <sys/types.h>
#include "io.h"
#include "process.h"
#include "env.h"

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
