#include "sys/mman.h"
#include <sys/syscall.h>
#include <unistd.h>

extern long syscall(long number, ...);

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    return (void *)syscall(SYS_mmap, addr, length, prot, flags, fd, offset);
}

int munmap(void *addr, size_t length)
{
    return (int)syscall(SYS_munmap, addr, length);
}
