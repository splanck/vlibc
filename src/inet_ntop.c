#include "arpa/inet.h"
#include "errno.h"
#include "stdio.h"
#include <arpa/inet.h>

const char *inet_ntop(int af, const void *src, char *dst, socklen_t size)
{
    if (af != AF_INET) {
        errno = EAFNOSUPPORT;
        return NULL;
    }
    if (!src || !dst || size == 0) {
        errno = EINVAL;
        return NULL;
    }
    if (size < INET_ADDRSTRLEN) {
        errno = ENOSPC;
        return NULL;
    }
    const struct in_addr *in = src;
    uint32_t addr = ntohl(in->s_addr);
    unsigned char a = (addr >> 24) & 0xFF;
    unsigned char b = (addr >> 16) & 0xFF;
    unsigned char c = (addr >> 8) & 0xFF;
    unsigned char d = addr & 0xFF;
    snprintf(dst, size, "%u.%u.%u.%u", a, b, c, d);
    return dst;
}
