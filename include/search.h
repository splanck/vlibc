/*
 * BSD 2-Clause License
 *
 * Purpose: Declarations for simple hash table search functions.
 */
#ifndef SEARCH_H
#define SEARCH_H

#include <stddef.h>

typedef struct entry {
    char *key;
    void *data;
} ENTRY;

typedef enum { FIND, ENTER } ACTION;

int hcreate(size_t nel);
void hdestroy(void);
ENTRY *hsearch(ENTRY item, ACTION action);

#endif /* SEARCH_H */
