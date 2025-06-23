#include "sys/socket.h"
#include "errno.h"
#include <sys/syscall.h>
#include <unistd.h>
#include "syscall.h"

int socketpair(int domain, int type, int protocol, int sv[2])
{
    long ret = vlibc_syscall(SYS_socketpair, domain, type, protocol,
                             (long)sv, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
}
