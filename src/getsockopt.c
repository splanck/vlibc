#include "sys/socket.h"
#include "errno.h"
#include <sys/syscall.h>
#include <unistd.h>
#include "syscall.h"

int getsockopt(int sockfd, int level, int optname,
               void *optval, socklen_t *optlen)
{
    long ret = vlibc_syscall(SYS_getsockopt, sockfd, level, optname,
                             (long)optval, (long)optlen, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return (int)ret;
}
