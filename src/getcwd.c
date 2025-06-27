/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms, with or without modification, are permitted provided that the copyright notice and this permission notice appear in all copies. This software is provided "as is" without warranty.
 *
 * Purpose: Implements the getcwd functions for vlibc. Provides wrappers and helpers used by the standard library.
 *
 * Copyright (c) 2025
 */

#include "stdlib.h"
#include "errno.h"

extern char *host_getcwd(char *buf, size_t size) __asm__("getcwd");

char *getcwd(char *buf, size_t size)
{
    if (buf) {
        if (size == 0) {
            errno = EINVAL;
            return NULL;
        }
        return host_getcwd(buf, size);
    }

    size_t cap = size ? size : 256;
    char *out = malloc(cap);
    if (!out)
        return NULL;

    for (;;) {
        char *res = host_getcwd(out, cap);
        if (res)
            return out;
        if (errno != ERANGE)
            break;
        size_t new_cap = cap * 2;
        char *tmp = realloc(out, new_cap);
        if (!tmp) {
            free(out);
            errno = ENOMEM;
            return NULL;
        }
        out = tmp;
        cap = new_cap;
    }
    free(out);
    return NULL;
}
