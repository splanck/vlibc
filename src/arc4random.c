/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms, with or without modification, are permitted provided that the copyright notice and this permission notice appear in all copies. This software is provided "as is" without warranty.
 *
 * Purpose: Implements the arc4random functions for vlibc. Provides wrappers and helpers used by the standard library.
 *
 * Copyright (c) 2025
 */

#include "stdlib.h"
#include "io.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

/*
 * Simple arc4random implementation. Use the system getrandom if available
 * in the future. For now read from /dev/urandom which is present on most
 * Unix-like systems. On BSD systems with arc4random in libc this fallback
 * keeps us portable when that function is absent.
 */

static void fill_urandom(void *buf, size_t len)
{
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0)
        return;
    size_t off = 0;
    while (off < len) {
        ssize_t r = read(fd, (unsigned char *)buf + off, len - off);
        if (r <= 0)
            break;
        off += (size_t)r;
    }
    close(fd);
}

void arc4random_buf(void *buf, size_t len)
{
    fill_urandom(buf, len);
}

unsigned int arc4random(void)
{
    unsigned int v = 0;
    arc4random_buf(&v, sizeof(v));
    return v;
}

int rand_r(unsigned *state)
{
    *state = *state * 1103515245 + 12345;
    return (int)((*state >> 16) & 0x7fff);
}
