#include <stdarg.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>

long vlibc_syscall(long number, ...)
{
    va_list ap;
    va_start(ap, number);
    unsigned long a1 = va_arg(ap, unsigned long);
    unsigned long a2 = va_arg(ap, unsigned long);
    unsigned long a3 = va_arg(ap, unsigned long);
    unsigned long a4 = va_arg(ap, unsigned long);
    unsigned long a5 = va_arg(ap, unsigned long);
    unsigned long a6 = va_arg(ap, unsigned long);
    va_end(ap);

    long ret = syscall(number, a1, a2, a3, a4, a5, a6);
    if (ret < 0)
        return -errno;
    return ret;
}
