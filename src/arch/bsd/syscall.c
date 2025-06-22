#include <stdarg.h>
#include <unistd.h>
#include <sys/syscall.h>
#include_next <errno.h>

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
    /*
     * POSIX semantics: syscall() returns -1 on failure and sets errno.
     * Negative values other than -1 are valid results for some calls,
     * so check explicitly for -1 and propagate errno as a negative code.
     */
    if (ret == -1) {
        int err = errno;
        return -err;
    }
    return ret;
}
