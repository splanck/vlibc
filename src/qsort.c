/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms, with or without modification, are permitted provided that the copyright notice and this permission notice appear in all copies. This software is provided "as is" without warranty.
 *
 * Purpose: Implements the qsort functions for vlibc. Provides wrappers and helpers used by the standard library.
 *
 * Copyright (c) 2025
 */

#include "stdlib.h"

static void swap(char *a, char *b, size_t size)
{
    while (size--) {
        char tmp = *a;
        *a++ = *b;
        *b++ = tmp;
    }
}

void qsort(void *base, size_t nmemb, size_t size,
           int (*compar)(const void *, const void *))
{
    char *b = base;
    for (size_t i = 0; i < nmemb; i++) {
        for (size_t j = i + 1; j < nmemb; j++) {
            char *pi = b + i * size;
            char *pj = b + j * size;
            if (compar(pi, pj) > 0)
                swap(pi, pj, size);
        }
    }
}

void qsort_r(void *base, size_t nmemb, size_t size,
             int (*compar)(const void *, const void *, void *), void *ctx)
{
    char *b = base;
    for (size_t i = 0; i < nmemb; i++) {
        for (size_t j = i + 1; j < nmemb; j++) {
            char *pi = b + i * size;
            char *pj = b + j * size;
            if (compar(pi, pj, ctx) > 0)
                swap(pi, pj, size);
        }
    }
}

void *bsearch(const void *key, const void *base, size_t nmemb, size_t size,
              int (*compar)(const void *, const void *))
{
    size_t low = 0;
    size_t high = nmemb;
    const char *b = base;

    while (low < high) {
        size_t mid = (low + high) / 2;
        const void *elem = b + mid * size;
        int c = compar(key, elem);
        if (c < 0)
            high = mid;
        else if (c > 0)
            low = mid + 1;
        else
            return (void *)elem;
    }
    return NULL;
}
