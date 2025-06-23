#include "sys/socket.h"
#include "errno.h"
#include <sys/syscall.h>
#include <unistd.h>
#include "syscall.h"

ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags)
{
#ifdef SYS_sendmsg
    long ret = vlibc_syscall(SYS_sendmsg, sockfd, (long)msg, flags, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (ssize_t)ret;
#else
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    extern ssize_t host_sendmsg(int, const struct msghdr *, int) __asm__("sendmsg");
    return host_sendmsg(sockfd, msg, flags);
#else
    (void)sockfd; (void)msg; (void)flags;
    errno = ENOSYS;
    return -1;
#endif
#endif
}

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags)
{
#ifdef SYS_recvmsg
    long ret = vlibc_syscall(SYS_recvmsg, sockfd, (long)msg, flags, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (ssize_t)ret;
#else
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    extern ssize_t host_recvmsg(int, struct msghdr *, int) __asm__("recvmsg");
    return host_recvmsg(sockfd, msg, flags);
#else
    (void)sockfd; (void)msg; (void)flags;
    errno = ENOSYS;
    return -1;
#endif
#endif
}

