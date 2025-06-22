#include "sys/socket.h"
#include "errno.h"
#include <sys/syscall.h>
#include <unistd.h>
#include "syscall.h"
#include "vlibc_features.h"

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
#if VLIBC_HAVE_ACCEPT4
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

int accept4(int sockfd, struct sockaddr *addr, socklen_t *addrlen, int flags)
{
#if VLIBC_HAVE_ACCEPT4
    long ret = vlibc_syscall(SYS_accept4, sockfd, (long)addr, (long)addrlen, flags, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
#else
    if (flags != 0) {
        errno = EINVAL;
        return -1;
    }
    return accept(sockfd, addr, addrlen);
#endif
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

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest, socklen_t addrlen)
{
    long ret = vlibc_syscall(SYS_sendto, sockfd, (long)buf, len, flags,
                             (long)dest, addrlen);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (ssize_t)ret;
}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                 struct sockaddr *src, socklen_t *addrlen)
{
    long ret = vlibc_syscall(SYS_recvfrom, sockfd, (long)buf, len, flags,
                             (long)src, (long)addrlen);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (ssize_t)ret;
}
