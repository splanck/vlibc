/*
 * BSD 2-Clause License
 *
 * Purpose: Memory mapping and protection interfaces.
 */
#ifndef MMAN_H
#define MMAN_H

#include <sys/types.h>
#include <stddef.h>

#if defined(__has_include)
#  if __has_include("/usr/include/x86_64-linux-gnu/sys/mman.h")
#    include "/usr/include/x86_64-linux-gnu/sys/mman.h"
#  elif __has_include("/usr/include/sys/mman.h")
#    include "/usr/include/sys/mman.h"
#  endif
#endif

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int munmap(void *addr, size_t length);
int mprotect(void *addr, size_t length, int prot);
int msync(void *addr, size_t length, int flags);

/*
 * Some BSD systems expose MAP_ANON instead of MAP_ANONYMOUS.  Provide
 * fallback aliases so callers can use either spelling regardless of the
 * underlying platform.
 */
#if defined(MAP_ANON) && !defined(MAP_ANONYMOUS)
#define MAP_ANONYMOUS MAP_ANON
#endif
#if defined(MAP_ANONYMOUS) && !defined(MAP_ANON)
#define MAP_ANON MAP_ANONYMOUS
#endif

/* Provide basic memory protection and mapping flags when missing. */
#ifndef PROT_READ
#define PROT_READ 0x1
#endif
#ifndef PROT_WRITE
#define PROT_WRITE 0x2
#endif
#ifndef PROT_EXEC
#define PROT_EXEC 0x4
#endif
#ifndef MAP_PRIVATE
#define MAP_PRIVATE 0x02
#endif
#ifndef MAP_FAILED
#define MAP_FAILED ((void *)-1)
#endif

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS 0x20
#endif
#ifndef MAP_ANON
#define MAP_ANON MAP_ANONYMOUS
#endif

#ifndef MS_SYNC
#define MS_SYNC 4
#endif
#ifndef MS_ASYNC
#define MS_ASYNC 1
#endif

#endif /* MMAN_H */
