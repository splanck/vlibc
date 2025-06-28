/*
 * BSD 2-Clause License
 *
 * Purpose: Declarations for user context manipulation.
 */
#ifndef UCONTEXT_H
#define UCONTEXT_H

#include "signal.h"
#include "setjmp.h"
#include <stdarg.h>

#ifndef VLIBC_HAS_SYS_UCONTEXT
typedef struct ucontext {
    struct ucontext *uc_link;
    stack_t          uc_stack;
    sigset_t         uc_sigmask;
    void (*uc_func)(void);
    int              uc_argc;
    long             uc_args[6];
    jmp_buf          __jmpbuf;
} ucontext_t;
#endif

int getcontext(ucontext_t *ucp);
int setcontext(const ucontext_t *ucp);
void makecontext(ucontext_t *ucp, void (*func)(void), int argc, ...);
int swapcontext(ucontext_t *oucp, const ucontext_t *ucp);

#endif /* UCONTEXT_H */
