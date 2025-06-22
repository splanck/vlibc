#include "memory.h"
#include "string.h"
#include <stdint.h>

#ifdef HAVE_SBRK
#include <unistd.h>
/* Declare sbrk for strict environments */
extern void *sbrk(intptr_t increment);

/*
 * Very small free list allocator. Each allocation stores a size header and
 * freed blocks are placed on a singly linked list so they can be reused by
 * subsequent malloc calls. Blocks are not coalesced or split which keeps the
 * implementation simple while still allowing memory to be recycled.
 */
struct block_header {
    size_t size;
    struct block_header *next; /* valid only when block is free */
};

static struct block_header *free_list = NULL;

void *malloc(size_t size)
{
    if (size == 0)
        return NULL;

    /* first-fit search through free list */
    struct block_header *prev = NULL;
    struct block_header *b = free_list;
    while (b) {
        if (b->size >= size) {
            if (prev)
                prev->next = b->next;
            else
                free_list = b->next;
            return (void *)(b + 1);
        }
        prev = b;
        b = b->next;
    }

    /* allocate new block from the system */
    struct block_header *hdr = sbrk(sizeof(struct block_header) + size);
    if (hdr == (void *)-1)
        return NULL;
    hdr->size = size;
    return (void *)(hdr + 1);
}

void free(void *ptr)
{
    if (!ptr)
        return;

    struct block_header *hdr = (struct block_header *)ptr - 1;
    hdr->next = free_list;
    free_list = hdr;
}

#else /* HAVE_SBRK */
#include <sys/mman.h>

struct mmap_header {
    size_t size;
};

void *malloc(size_t size)
{
    if (size == 0)
        return NULL;

    size_t total = sizeof(struct mmap_header) + size;
    struct mmap_header *hdr = mmap(NULL, total, PROT_READ | PROT_WRITE,
                                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (hdr == MAP_FAILED)
        return NULL;

    hdr->size = size;
    return (void *)(hdr + 1);
}

void free(void *ptr)
{
    if (!ptr)
        return;

    struct mmap_header *hdr = (struct mmap_header *)ptr - 1;
    munmap(hdr, hdr->size + sizeof(struct mmap_header));
}

#endif /* HAVE_SBRK */

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

    if (size == 0) {
        free(ptr);
        return NULL;
    }

#ifdef HAVE_SBRK
    struct block_header *old_hdr = (struct block_header *)ptr - 1;
#else
    struct mmap_header *old_hdr = (struct mmap_header *)ptr - 1;
#endif
    size_t copy = old_hdr->size < size ? old_hdr->size : size;

    void *new_ptr = malloc(size);
    if (!new_ptr)
        return NULL;
    vmemcpy(new_ptr, ptr, copy);
    free(ptr);
    return new_ptr;
}
