/*
 * BSD 2-Clause License
 *
 * Purpose: System V IPC key generation.
 */
#ifndef SYS_IPC_H
#define SYS_IPC_H

#include <sys/types.h>

#if defined(__has_include)
#  if __has_include("/usr/include/x86_64-linux-gnu/sys/ipc.h")
#    include "/usr/include/x86_64-linux-gnu/sys/ipc.h"
#    define VLIBC_SYS_IPC_NATIVE 1
#  elif __has_include("/usr/include/sys/ipc.h")
#    include "/usr/include/sys/ipc.h"
#    define VLIBC_SYS_IPC_NATIVE 1
#  endif
#endif

#ifndef VLIBC_SYS_IPC_NATIVE
key_t ftok(const char *path, int proj_id);
/* Generate an IPC key from a file path and project id. */
#endif

#endif /* SYS_IPC_H */
