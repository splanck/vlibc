/*
 * BSD 2-Clause License
 *
 * Purpose: Implements hcreate, hdestroy and hsearch using
 * a simple open-addressed hash table.
 */

#include "search.h"
#include "stdlib.h"
#include "string.h"
#include "memory.h"

static ENTRY *table;
static unsigned char *used;
static size_t table_size;
static size_t items;

static unsigned long hash_str(const char *s)
{
    unsigned long h = 5381;
    unsigned char c;
    while ((c = (unsigned char)*s++))
        h = ((h << 5) + h) + c;
    return h;
}

int hcreate(size_t nel)
{
    if (table)
        return 0;
    table = calloc(nel, sizeof(ENTRY));
    if (!table)
        return 0;
    used = calloc(nel, 1);
    if (!used) {
        free(table);
        table = NULL;
        return 0;
    }
    table_size = nel;
    items = 0;
    return 1;
}

void hdestroy(void)
{
    free(table);
    free(used);
    table = NULL;
    used = NULL;
    table_size = 0;
    items = 0;
}

ENTRY *hsearch(ENTRY item, ACTION action)
{
    if (!table || !item.key)
        return NULL;
    unsigned long h = hash_str(item.key);
    for (size_t i = 0; i < table_size; i++) {
        size_t idx = (h + i) % table_size;
        if (!used[idx]) {
            if (action == ENTER) {
                if (items >= table_size)
                    return NULL;
                table[idx] = item;
                used[idx] = 1;
                items++;
                return &table[idx];
            }
            return NULL;
        }
        if (strcmp(table[idx].key, item.key) == 0)
            return &table[idx];
    }
    return NULL;
}
