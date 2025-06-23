#ifndef SYS_SOCKET_H
#define SYS_SOCKET_H

#include <sys/types.h>
#include <stddef.h>

#if defined(__has_include)
#  if __has_include("/usr/include/x86_64-linux-gnu/sys/socket.h")
#    include "/usr/include/x86_64-linux-gnu/sys/socket.h"
#  elif __has_include("/usr/include/sys/socket.h")
#    include "/usr/include/sys/socket.h"
#  endif
#endif

/* Socket API wrappers */
int socket(int domain, int type, int protocol);
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int listen(int sockfd, int backlog);
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int accept4(int sockfd, struct sockaddr *addr, socklen_t *addrlen, int flags);
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int socketpair(int domain, int type, int protocol, int sv[2]);
ssize_t send(int sockfd, const void *buf, size_t len, int flags);
ssize_t recv(int sockfd, void *buf, size_t len, int flags);
ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest, socklen_t addrlen);
ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                 struct sockaddr *src, socklen_t *addrlen);
int setsockopt(int sockfd, int level, int optname,
               const void *optval, socklen_t optlen);
int getsockopt(int sockfd, int level, int optname,
               void *optval, socklen_t *optlen);

#endif /* SYS_SOCKET_H */
