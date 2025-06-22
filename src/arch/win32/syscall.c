#include <stdarg.h>
#include_next <errno.h>
#include "syscall.h"

long vlibc_syscall(long number, ...)
{
    (void)number;
    errno = ENOSYS;
    return -1;
}
