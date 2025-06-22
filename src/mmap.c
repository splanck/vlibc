#include "sys/mman.h"
#include "errno.h"
#include_next <sys/mman.h>

/*
 * Call the system implementations of mmap(2), munmap(2) and mprotect(2)
 * rather than issuing raw syscalls directly.  On BSD the raw syscall
 * interface may differ from Linux, so using the libc functions ensures
 * compatibility.
 */

/* resolve the C library's mmap implementations dynamically to avoid
 * recursive self-calls when the vlibc versions share the same symbol
 * names. */
extern void *host_mmap(void *, size_t, int, int, int, off_t) __asm__("mmap");
extern int host_munmap(void *, size_t) __asm__("munmap");
extern int host_mprotect(void *, size_t, int) __asm__("mprotect");

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    return host_mmap(addr, length, prot, flags, fd, offset);
}

int munmap(void *addr, size_t length)
{
    return host_munmap(addr, length);
}

int mprotect(void *addr, size_t length, int prot)
{
    return host_mprotect(addr, length, prot);
}
