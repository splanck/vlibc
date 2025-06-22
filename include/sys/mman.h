#ifndef MMAN_H
#define MMAN_H

#include <sys/types.h>
#include <stddef.h>

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int munmap(void *addr, size_t length);
int mprotect(void *addr, size_t length, int prot);

#endif /* MMAN_H */

#include_next <sys/mman.h>

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
