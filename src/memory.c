#include "memory.h"
#include <unistd.h>
#include "string.h"
#include <stdint.h>

/* Declare sbrk for strict environments */
extern void *sbrk(intptr_t increment);

/*
 * Extremely small bump allocator. Each allocation stores a size header so
 * the most recently allocated block can be released. Memory is only
 * returned to the system if free() is called on the last allocation.
 * This remains simple and suitable for tiny programs or testing but is
 * not intended for long-running processes.
 */

static char *heap_end = NULL;

/* Size header stored before each allocation */
struct block_header {
    size_t size;
};

void *malloc(size_t size)
{
    if (size == 0)
        return NULL;

    if (!heap_end)
        heap_end = sbrk(0);

    /* allocate space for header + requested size */
    size_t total = sizeof(struct block_header) + size;

    char *prev = heap_end;
    if (sbrk(total) == (void *)-1)
        return NULL;

    /* store header and move heap pointer */
    struct block_header *hdr = (struct block_header *)prev;
    hdr->size = size;

    heap_end += total;
    return (void *)(hdr + 1);
}

void free(void *ptr)
{
    if (!ptr)
        return;

    struct block_header *hdr = (struct block_header *)ptr - 1;
    size_t total = sizeof(struct block_header) + hdr->size;

    /* only release memory if this is the most recent allocation */
    if ((char *)ptr + hdr->size == heap_end) {
        if (sbrk(-((intptr_t)total)) != (void *)-1)
            heap_end -= total;
    }
}

void *calloc(size_t nmemb, size_t size)
{
    size_t total = nmemb * size;
    void *ptr = malloc(total);
    if (ptr)
        vmemset(ptr, 0, total);
    return ptr;
}

void *realloc(void *ptr, size_t size)
{
    if (!ptr)
        return malloc(size);

    void *new_ptr = malloc(size);
    if (new_ptr && size > 0)
        vmemmove(new_ptr, ptr, size);
    return new_ptr;
}
