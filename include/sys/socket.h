#ifndef SYS_SOCKET_H
#define SYS_SOCKET_H

#include <sys/types.h>
#include <stdint.h>
#include <stddef.h>

/* Basic socket address structures */

typedef uint16_t sa_family_t;
typedef uint16_t in_port_t;
typedef uint32_t in_addr_t;
typedef unsigned int socklen_t;

/* Common address families */
#define AF_UNSPEC 0
#define AF_UNIX   1
#define AF_LOCAL  1
#define AF_INET   2
#define AF_INET6  10

/* Socket types */
#define SOCK_STREAM 1
#define SOCK_DGRAM  2

struct sockaddr {
    sa_family_t sa_family;
    char sa_data[14];
};

struct in_addr {
    in_addr_t s_addr;
};

struct sockaddr_in {
    sa_family_t sin_family;
    in_port_t sin_port;
    struct in_addr sin_addr;
    unsigned char sin_zero[8];
};

/* Socket API wrappers */
int socket(int domain, int type, int protocol);
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int listen(int sockfd, int backlog);
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
ssize_t send(int sockfd, const void *buf, size_t len, int flags);
ssize_t recv(int sockfd, void *buf, size_t len, int flags);

#endif /* SYS_SOCKET_H */
