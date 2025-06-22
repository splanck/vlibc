#include "arpa/inet.h"
#include "errno.h"
#include "stdlib.h"
#include <stdint.h>
#include <arpa/inet.h>

static int parse_ipv4(const char *s, uint32_t *out)
{
    char *end;
    long a = strtol(s, &end, 10);
    if (a < 0 || a > 255 || *end != '.')
        return -1;
    s = end + 1;
    long b = strtol(s, &end, 10);
    if (b < 0 || b > 255 || *end != '.')
        return -1;
    s = end + 1;
    long c = strtol(s, &end, 10);
    if (c < 0 || c > 255 || *end != '.')
        return -1;
    s = end + 1;
    long d = strtol(s, &end, 10);
    if (d < 0 || d > 255 || (*end && *end != ' ' && *end != '\t'))
        return -1;
    *out = ((uint32_t)a << 24) | ((uint32_t)b << 16) |
           ((uint32_t)c << 8) | (uint32_t)d;
    return 0;
}

int inet_pton(int af, const char *src, void *dst)
{
    if (af != AF_INET) {
        errno = EAFNOSUPPORT;
        return -1;
    }
    if (!src || !dst) {
        errno = EINVAL;
        return -1;
    }
    uint32_t ip;
    if (parse_ipv4(src, &ip) != 0)
        return 0;
    struct in_addr *in = dst;
    in->s_addr = htonl(ip);
    return 1;
}
