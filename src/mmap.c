#include "sys/mman.h"
#include "errno.h"
#include <sys/syscall.h>
#include <unistd.h>
#include "syscall.h"

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    long ret = vlibc_syscall(SYS_mmap, (long)addr, length, prot, flags, fd, offset);
    if (ret < 0) {
        errno = -ret;
        return (void *)-1;
    }
    return (void *)ret;
}

int munmap(void *addr, size_t length)
{
    long ret = vlibc_syscall(SYS_munmap, (long)addr, length, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
}

int mprotect(void *addr, size_t length, int prot)
{
    long ret = vlibc_syscall(SYS_mprotect, (long)addr, length, prot, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
}
