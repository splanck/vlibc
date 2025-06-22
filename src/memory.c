#include "memory.h"
#include <unistd.h>
#include "string.h"
#include <stdint.h>

/* Declare sbrk for strict environments */
extern void *sbrk(intptr_t increment);

/*
 * Very small free list allocator. Each allocation stores a size header and
 * freed blocks are placed on a singly linked list so they can be reused by
 * subsequent malloc calls.  Freed blocks are kept sorted by address and
 * adjacent regions are coalesced to limit fragmentation.  Blocks may be split
 * when a smaller request fits inside a larger free region.
 */

struct block_header {
    size_t size;
    struct block_header *next; /* valid only when block is free */
};

static struct block_header *free_list = NULL;

static size_t align_up(size_t n)
{
    size_t a = sizeof(size_t);
    return (n + a - 1) & ~(a - 1);
}

static void insert_block(struct block_header *hdr)
{
    struct block_header *prev = NULL;
    struct block_header *cur = free_list;

    while (cur && cur < hdr) {
        prev = cur;
        cur = cur->next;
    }

    hdr->next = cur;
    if (prev)
        prev->next = hdr;
    else
        free_list = hdr;

    /* coalesce with next */
    if (hdr->next &&
        (char *)hdr + sizeof(struct block_header) + hdr->size ==
        (char *)hdr->next) {
        hdr->size += sizeof(struct block_header) + hdr->next->size;
        hdr->next = hdr->next->next;
    }

    /* coalesce with previous */
    if (prev &&
        (char *)prev + sizeof(struct block_header) + prev->size ==
        (char *)hdr) {
        prev->size += sizeof(struct block_header) + hdr->size;
        prev->next = hdr->next;
    }
}

void *malloc(size_t size)
{
    if (size == 0)
        return NULL;

    size = align_up(size);

    /* first-fit search through free list */
    struct block_header **prev = &free_list;
    struct block_header *b = free_list;
    while (b) {
        if (b->size >= size) {
            if (b->size >= size + sizeof(struct block_header) + sizeof(size_t)) {
                struct block_header *split = (struct block_header *)((char *)(b + 1) + size);
                split->size = b->size - size - sizeof(struct block_header);
                split->next = b->next;
                *prev = split;
                b->size = size;
            } else {
                *prev = b->next;
            }
            return (void *)(b + 1);
        }
        prev = &b->next;
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
    insert_block(hdr);
}

void *calloc(size_t nmemb, size_t size)
{
    size_t total;
    if (__builtin_mul_overflow(nmemb, size, &total))
        return NULL;
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

    size = align_up(size);

    struct block_header *old_hdr = (struct block_header *)ptr - 1;

    if (old_hdr->size >= size) {
        if (old_hdr->size >= size + sizeof(struct block_header) + sizeof(size_t)) {
            struct block_header *split = (struct block_header *)((char *)(old_hdr + 1) + size);
            split->size = old_hdr->size - size - sizeof(struct block_header);
            old_hdr->size = size;
            insert_block(split);
        }
        return ptr;
    }

    size_t copy = old_hdr->size < size ? old_hdr->size : size;

    void *new_ptr = malloc(size);
    if (!new_ptr)
        return NULL;
    vmemcpy(new_ptr, ptr, copy);
    free(ptr);
    return new_ptr;
}
