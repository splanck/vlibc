/*
 * BSD 2-Clause License
 *
 * Purpose: POSIX shared memory interfaces.
 */
#ifndef SYS_SHM_H
#define SYS_SHM_H

#include <sys/types.h>

#if defined(__has_include)
#  if __has_include("/usr/include/x86_64-linux-gnu/sys/shm.h")
#    include "/usr/include/x86_64-linux-gnu/sys/shm.h"
#    define VLIBC_SYS_SHM_NATIVE 1
#  elif __has_include("/usr/include/sys/shm.h")
#    include "/usr/include/sys/shm.h"
#    define VLIBC_SYS_SHM_NATIVE 1
#  endif
#endif

#ifndef VLIBC_SYS_SHM_NATIVE
int shm_open(const char *name, int oflag, mode_t mode);
/* Open or create a POSIX shared memory object. */
int shm_unlink(const char *name);
/* Remove a POSIX shared memory object. */
#endif

#endif /* SYS_SHM_H */
