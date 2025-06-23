#include "sys/socket.h"
#include "errno.h"
#include <sys/syscall.h>
#include <unistd.h>
#include "syscall.h"

int setsockopt(int sockfd, int level, int optname,
               const void *optval, socklen_t optlen)
{
    long ret = vlibc_syscall(SYS_setsockopt, sockfd, level, optname,
                             (long)optval, optlen, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
}
