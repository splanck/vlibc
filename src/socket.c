#include "sys/socket.h"
#include "errno.h"
#include <sys/syscall.h>
#include <unistd.h>
#include "syscall.h"

int socket(int domain, int type, int protocol)
{
    long ret = vlibc_syscall(SYS_socket, domain, type, protocol, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
}

int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    long ret = vlibc_syscall(SYS_bind, sockfd, (long)addr, addrlen, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
}

int listen(int sockfd, int backlog)
{
    long ret = vlibc_syscall(SYS_listen, sockfd, backlog, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
#ifdef SYS_accept4
    long ret = vlibc_syscall(SYS_accept4, sockfd, (long)addr, (long)addrlen, 0, 0, 0);
#else
    long ret = vlibc_syscall(SYS_accept, sockfd, (long)addr, (long)addrlen, 0, 0, 0);
#endif
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    long ret = vlibc_syscall(SYS_connect, sockfd, (long)addr, addrlen, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
}

ssize_t send(int sockfd, const void *buf, size_t len, int flags)
{
    long ret = vlibc_syscall(SYS_sendto, sockfd, (long)buf, len, flags, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (ssize_t)ret;
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags)
{
    long ret = vlibc_syscall(SYS_recvfrom, sockfd, (long)buf, len, flags, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (ssize_t)ret;
}
