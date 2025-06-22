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
#include "dlfcn.h"
#ifndef RTLD_NEXT
#define RTLD_NEXT ((void *)-1)
#endif

static void *(*host_mmap)(void *, size_t, int, int, int, off_t);
static int (*host_munmap)(void *, size_t);
static int (*host_mprotect)(void *, size_t, int);

__attribute__((constructor))
static void init_mmap_syms(void)
{
    host_mmap = dlsym(RTLD_NEXT, "mmap");
    host_munmap = dlsym(RTLD_NEXT, "munmap");
    host_mprotect = dlsym(RTLD_NEXT, "mprotect");
}

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
