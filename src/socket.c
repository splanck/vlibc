#include "sys/socket.h"
#include <sys/syscall.h>
#include <unistd.h>

extern long syscall(long number, ...);

int socket(int domain, int type, int protocol)
{
    return (int)syscall(SYS_socket, domain, type, protocol);
}

int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    return (int)syscall(SYS_bind, sockfd, addr, addrlen);
}

int listen(int sockfd, int backlog)
{
    return (int)syscall(SYS_listen, sockfd, backlog);
}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
#ifdef SYS_accept4
    return (int)syscall(SYS_accept4, sockfd, addr, addrlen, 0);
#else
    return (int)syscall(SYS_accept, sockfd, addr, addrlen);
#endif
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    return (int)syscall(SYS_connect, sockfd, addr, addrlen);
}

ssize_t send(int sockfd, const void *buf, size_t len, int flags)
{
    return (ssize_t)syscall(SYS_sendto, sockfd, buf, len, flags, NULL, 0);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags)
{
    return (ssize_t)syscall(SYS_recvfrom, sockfd, buf, len, flags, NULL, NULL);
}
