#include "memory.h"
#include <unistd.h>
#include <string.h>
#include <stdint.h>

/* Declare sbrk for strict environments */
extern void *sbrk(intptr_t increment);

/*
 * Extremely small bump allocator. Memory obtained via sbrk is never
 * returned to the system. free() is therefore a stub. This is suitable
 * for tiny programs or testing but not for long-running processes.
 */

static char *heap_end = NULL;

void *malloc(size_t size)
{
    if (size == 0)
        return NULL;

    if (!heap_end)
        heap_end = sbrk(0);

    char *prev = heap_end;
    if (sbrk(size) == (void *)-1)
        return NULL;

    heap_end += size;
    return prev;
}

void free(void *ptr)
{
    (void)ptr; /* no-op */
}

void *calloc(size_t nmemb, size_t size)
{
    size_t total = nmemb * size;
    void *ptr = malloc(total);
    if (ptr)
        memset(ptr, 0, total);
    return ptr;
}

void *realloc(void *ptr, size_t size)
{
    if (!ptr)
        return malloc(size);

    void *new_ptr = malloc(size);
    if (new_ptr && size > 0)
        memmove(new_ptr, ptr, size);
    return new_ptr;
}
