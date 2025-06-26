/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms, with
 * or without modification, are permitted provided that the copyright notice and
 * this permission notice appear in all copies. This software is provided "as is" without warranty.
 *
 * Purpose: Implements sigsetjmp and siglongjmp for vlibc.
 */

#include "setjmp.h"
#include <setjmp.h>
#include "signal.h"

#ifdef setjmp
#undef setjmp
#endif
#ifdef longjmp
#undef longjmp
#endif
#ifdef sigsetjmp
#undef sigsetjmp
#endif
#ifdef siglongjmp
#undef siglongjmp
#endif

int sigsetjmp(sigjmp_buf env, int save)
{
    if (save) {
        env[0].__mask_was_saved = 1;
        sigprocmask(SIG_BLOCK, NULL, &env[0].__saved_mask);
    } else {
        env[0].__mask_was_saved = 0;
    }
    return setjmp(env);
}

void siglongjmp(sigjmp_buf env, int val)
{
    if (env[0].__mask_was_saved)
        sigprocmask(SIG_SETMASK, &env[0].__saved_mask, NULL);
    longjmp(env, val);
}
