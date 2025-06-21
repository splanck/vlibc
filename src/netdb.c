#include "netdb.h"
#include "memory.h"
#include "string.h"
#include "io.h"
#include "errno.h"
#include "stdlib.h"
#include "stdio.h"

#ifndef O_RDONLY
#define O_RDONLY 0
#endif

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
    *out = (uint32_t)((a << 24) | (b << 16) | (c << 8) | d);
    return 0;
}

static int hosts_lookup(const char *node, uint32_t *ip)
{
    int fd = open("/etc/hosts", O_RDONLY, 0);
    if (fd < 0)
        return -1;

    char buf[2048];
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (n <= 0)
        return -1;
    buf[n] = '\0';

    char *saveptr1;
    for (char *line = strtok_r(buf, "\n", &saveptr1); line; line = strtok_r(NULL, "\n", &saveptr1)) {
        while (*line == ' ' || *line == '\t')
            line++;
        if (*line == '#' || *line == '\0')
            continue;
        char *saveptr2;
        char *tok = strtok_r(line, " \t", &saveptr2);
        if (!tok)
            continue;
        uint32_t addr;
        if (parse_ipv4(tok, &addr) != 0)
            continue;
        while ((tok = strtok_r(NULL, " \t", &saveptr2))) {
            if (strcmp(tok, node) == 0) {
                *ip = addr;
                return 0;
            }
        }
    }
    return -1;
}

static uint16_t htons16(uint16_t v)
{
    return (uint16_t)(((v & 0xFF) << 8) | ((v >> 8) & 0xFF));
}

static uint16_t ntohs16(uint16_t v)
{
    return htons16(v);
}

int getaddrinfo(const char *node, const char *service,
                const struct addrinfo *hints, struct addrinfo **res)
{
    (void)hints;
    if (!node && !service)
        return -1;

    uint32_t ip = 0;
    if (node) {
        if (parse_ipv4(node, &ip) != 0) {
            if (hosts_lookup(node, &ip) != 0) {
                errno = ENOENT;
                return -1;
            }
        }
    }

    uint16_t port = 0;
    if (service)
        port = (uint16_t)atoi(service);

    struct sockaddr_in *sa = malloc(sizeof(struct sockaddr_in));
    if (!sa)
        return -1;
    sa->sin_family = AF_INET;
    sa->sin_port = htons16(port);
    sa->sin_addr.s_addr = ip;
    memset(sa->sin_zero, 0, sizeof(sa->sin_zero));

    struct addrinfo *ai = malloc(sizeof(struct addrinfo));
    if (!ai) {
        free(sa);
        return -1;
    }
    ai->ai_flags = 0;
    ai->ai_family = AF_INET;
    ai->ai_socktype = 0;
    ai->ai_protocol = 0;
    ai->ai_addrlen = sizeof(struct sockaddr_in);
    ai->ai_addr = (struct sockaddr *)sa;
    ai->ai_canonname = NULL;
    ai->ai_next = NULL;

    *res = ai;
    return 0;
}

void freeaddrinfo(struct addrinfo *res)
{
    while (res) {
        struct addrinfo *next = res->ai_next;
        if (res->ai_addr)
            free(res->ai_addr);
        if (res->ai_canonname)
            free(res->ai_canonname);
        free(res);
        res = next;
    }
}

int getnameinfo(const struct sockaddr *sa, socklen_t salen,
                char *host, socklen_t hostlen,
                char *serv, socklen_t servlen, int flags)
{
    (void)flags;
    if (!sa || salen < (socklen_t)sizeof(struct sockaddr_in))
        return -1;
    const struct sockaddr_in *sin = (const struct sockaddr_in *)sa;
    if (host && hostlen > 0) {
        unsigned char b[4];
        uint32_t addr = sin->sin_addr.s_addr;
        b[0] = (addr >> 24) & 0xFF;
        b[1] = (addr >> 16) & 0xFF;
        b[2] = (addr >> 8) & 0xFF;
        b[3] = addr & 0xFF;
        snprintf(host, hostlen, "%u.%u.%u.%u", b[0], b[1], b[2], b[3]);
    }
    if (serv && servlen > 0)
        snprintf(serv, servlen, "%u", ntohs16(sin->sin_port));
    return 0;
}
