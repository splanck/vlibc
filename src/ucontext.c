/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms,
 * with or without modification, are permitted provided that the copyright
 * notice and this permission notice appear in all copies. This software is
 * provided "as is" without warranty.
 *
 * Purpose: Implements the ucontext functions for vlibc.
 */

#include "ucontext.h"
#include "stdlib.h"
#include "string.h"
#include "errno.h"
#include "process.h"
#include <stdarg.h>

/* trampoline for newly created contexts */
static void __attribute__((noreturn)) ctx_trampoline(void)
{
    ucontext_t *uc;
    __asm__ volatile("mov %%r12,%0" : "=r"(uc));

    void (*fn)(void) = uc->uc_func;
    long *a = uc->uc_args;
    switch (uc->uc_argc) {
    case 0: ((void (*)(void))fn)(); break;
    case 1: ((void (*)(long))fn)(a[0]); break;
    case 2: ((void (*)(long,long))fn)(a[0], a[1]); break;
    case 3: ((void (*)(long,long,long))fn)(a[0], a[1], a[2]); break;
    case 4: ((void (*)(long,long,long,long))fn)(a[0], a[1], a[2], a[3]); break;
    case 5: ((void (*)(long,long,long,long,long))fn)(a[0], a[1], a[2], a[3], a[4]); break;
    default:
        ((void (*)(long,long,long,long,long,long))fn)(a[0], a[1], a[2], a[3], a[4], a[5]);
        break;
    }

    if (uc->uc_link)
        setcontext(uc->uc_link);
    exit(0);
}

int getcontext(ucontext_t *ucp)
{
    if (!ucp) {
        errno = EINVAL;
        return -1;
    }
    sigprocmask(SIG_SETMASK, NULL, &ucp->uc_sigmask);
    if (setjmp(ucp->__jmpbuf) != 0)
        return 0;
    return 0;
}

int setcontext(const ucontext_t *ucp)
{
    if (!ucp) {
        errno = EINVAL;
        return -1;
    }
    sigprocmask(SIG_SETMASK, &ucp->uc_sigmask, NULL);
    longjmp(ucp->__jmpbuf, 1);
    __builtin_unreachable();
}

void makecontext(ucontext_t *ucp, void (*func)(void), int argc, ...)
{
    if (!ucp || !func)
        return;
    if (argc < 0)
        argc = 0;
    if (argc > 6)
        argc = 6;
    ucp->uc_func = func;
    ucp->uc_argc = argc;
    va_list ap;
    va_start(ap, argc);
    for (int i = 0; i < argc; i++)
        ucp->uc_args[i] = va_arg(ap, long);
    va_end(ap);

    char *sp = (char *)ucp->uc_stack.ss_sp + ucp->uc_stack.ss_size;
    sp = (char *)((uintptr_t)sp & ~15UL);
    setjmp(ucp->__jmpbuf);
    ((long *)ucp->__jmpbuf)[6] = (long)sp;
    ((long *)ucp->__jmpbuf)[7] = (long)ctx_trampoline;
    ((long *)ucp->__jmpbuf)[2] = (long)ucp; /* r12 */
}

int swapcontext(ucontext_t *oucp, const ucontext_t *ucp)
{
    if (getcontext(oucp) == 0)
        return setcontext(ucp);
    return 0;
}
