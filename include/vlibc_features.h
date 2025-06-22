#ifndef VLIBC_FEATURES_H
#define VLIBC_FEATURES_H

#include <sys/syscall.h>

#ifdef SYS_accept4
#define VLIBC_HAVE_ACCEPT4 1
#endif

#ifdef SYS_pipe2
#define VLIBC_HAVE_PIPE2 1
#endif

#ifdef SYS_dup3
#define VLIBC_HAVE_DUP3 1
#endif

#endif /* VLIBC_FEATURES_H */
