#include <stdarg.h>
#include <stdint.h>

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

    register unsigned long r10 __asm__("r10") = a4;
    register unsigned long r8 __asm__("r8") = a5;
    register unsigned long r9 __asm__("r9") = a6;
    unsigned long ret;
    __asm__ volatile("syscall"
                     : "=a"(ret)
                     : "a"(number), "D"(a1), "S"(a2), "d"(a3),
                       "r"(r10), "r"(r8), "r"(r9)
                     : "rcx", "r11", "memory");
    return (long)ret;
}
