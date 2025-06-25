/*
 * BSD 2-Clause License
 *
 * Purpose: Declarations for heap allocation functions.
 */
#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>

/* Allocate memory from free list or via sbrk/mmap. */
void *malloc(size_t size);
/* Release memory and return it to the allocator or unmap. */
void free(void *ptr);
/* Allocate zero-filled array using malloc. */
void *calloc(size_t nmemb, size_t size);
/* Resize a block by allocating and copying. */
void *realloc(void *ptr, size_t size);
/* Resize an array with overflow check. */
void *reallocarray(void *ptr, size_t nmemb, size_t size);
/* Resize an array and zero any newly allocated memory. */
void *recallocarray(void *ptr, size_t nmemb, size_t size);
/* Allocate aligned memory and record the original pointer. */
int posix_memalign(void **memptr, size_t alignment, size_t size);

#endif /* MEMORY_H */
