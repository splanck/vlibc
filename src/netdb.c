#include "netdb.h"
#include "memory.h"
#include "string.h"
#include "io.h"
#include "errno.h"
#include "stdlib.h"
#include "stdio.h"
#include "arpa/inet.h"
#include <stdint.h>
#include <netinet/in.h>

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

static int parse_ipv6(const char *s, unsigned char *out)
{
    uint16_t words[8] = {0};
    int cur = 0;
    int zpos = -1;
    if (*s == ':') {
        if (s[1] != ':')
            return -1;
        zpos = cur;
        s += 2;
    }
    while (*s && cur < 8) {
        if (*s == ':') {
            if (zpos != -1)
                return -1;
            zpos = cur;
            s++;
            if (*s == ':')
                return -1;
            continue;
        }
        char *end;
        long val = strtol(s, &end, 16);
        if (val < 0 || val > 0xffff)
            return -1;
        words[cur++] = (uint16_t)val;
        s = end;
        if (*s == '.') {
            if (cur > 6)
                return -1;
            uint32_t ipv4;
            if (parse_ipv4(s, &ipv4) != 0)
                return -1;
            words[cur++] = (uint16_t)((ipv4 >> 16) & 0xffff);
            words[cur++] = (uint16_t)(ipv4 & 0xffff);
            s += strlen(s);
            break;
        }
        if (*s == ':')
            s++;
        else if (*s)
            return -1;
    }
    if (*s)
        return -1;
    int fill = 8 - cur;
    if (zpos == -1 && cur != 8)
        return -1;
    if (zpos != -1) {
        for (int i = cur - 1; i >= zpos; i--)
            words[i + fill] = words[i];
        for (int i = zpos; i < zpos + fill; i++)
            words[i] = 0;
    }
    for (int i = 0; i < 8; i++) {
        out[i * 2] = words[i] >> 8;
        out[i * 2 + 1] = words[i] & 0xff;
    }
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

    uint32_t ip4 = 0;
    unsigned char ip6[16];
    int family = AF_UNSPEC;
    if (node) {
        if (parse_ipv6(node, ip6) == 0) {
            family = AF_INET6;
        } else if (parse_ipv4(node, &ip4) == 0) {
            family = AF_INET;
        } else {
            if (hosts_lookup(node, &ip4) != 0) {
                errno = ENOENT;
                return -1;
            }
            family = AF_INET;
        }
    } else {
        family = AF_INET;
    }

    uint16_t port = 0;
    if (service)
        port = (uint16_t)atoi(service);

    struct addrinfo *ai = malloc(sizeof(struct addrinfo));
    if (!ai)
        return -1;
    ai->ai_flags = 0;
    ai->ai_family = family;
    ai->ai_socktype = 0;
    ai->ai_protocol = 0;
    ai->ai_next = NULL;

    if (family == AF_INET6) {
        struct sockaddr_in6 *sa6 = malloc(sizeof(struct sockaddr_in6));
        if (!sa6) {
            free(ai);
            return -1;
        }
        sa6->sin6_family = AF_INET6;
        sa6->sin6_port = htons16(port);
        memcpy(&sa6->sin6_addr, ip6, 16);
        sa6->sin6_flowinfo = 0;
        sa6->sin6_scope_id = 0;
        ai->ai_addrlen = sizeof(struct sockaddr_in6);
        ai->ai_addr = (struct sockaddr *)sa6;
    } else {
        struct sockaddr_in *sa = malloc(sizeof(struct sockaddr_in));
        if (!sa) {
            free(ai);
            return -1;
        }
        sa->sin_family = AF_INET;
        sa->sin_port = htons16(port);
        sa->sin_addr.s_addr = ip4;
        memset(sa->sin_zero, 0, sizeof(sa->sin_zero));
        ai->ai_addrlen = sizeof(struct sockaddr_in);
        ai->ai_addr = (struct sockaddr *)sa;
    }
    ai->ai_canonname = NULL;

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
    if (!sa)
        return -1;
    if (sa->sa_family == AF_INET) {
        if (salen < (socklen_t)sizeof(struct sockaddr_in))
            return -1;
        const struct sockaddr_in *sin = (const struct sockaddr_in *)sa;
        if (host && hostlen > 0)
            inet_ntop(AF_INET, &sin->sin_addr, host, hostlen);
        if (serv && servlen > 0)
            snprintf(serv, servlen, "%u", ntohs16(sin->sin_port));
        return 0;
    } else if (sa->sa_family == AF_INET6) {
        if (salen < (socklen_t)sizeof(struct sockaddr_in6))
            return -1;
        const struct sockaddr_in6 *sin6 = (const struct sockaddr_in6 *)sa;
        if (host && hostlen > 0)
            inet_ntop(AF_INET6, &sin6->sin6_addr, host, hostlen);
        if (serv && servlen > 0)
            snprintf(serv, servlen, "%u", ntohs16(sin6->sin6_port));
        return 0;
    }
    return -1;
}
