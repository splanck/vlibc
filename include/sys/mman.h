#ifndef MMAN_H
#define MMAN_H

#include <sys/types.h>
#include <stddef.h>

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int munmap(void *addr, size_t length);

#endif /* MMAN_H */
