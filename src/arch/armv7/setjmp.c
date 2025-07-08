/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms, with
 * or without modification, are permitted provided that the copyright notice and this permission notice appear in all copies. This software is provided "as is" without warranty.
 *
 * Purpose: Implements the setjmp functions for vlibc on armv7 using
 * compiler builtins.
 */

#include "setjmp.h"
#include <setjmp.h>

#ifdef setjmp
#undef setjmp
#endif
#ifdef longjmp
#undef longjmp
#endif

int setjmp(jmp_buf env)
{
    return __builtin_setjmp(env);
}

void longjmp(jmp_buf env, int val)
{
    if (val == 0)
        val = 1;
    __builtin_longjmp(env, val);
    __builtin_unreachable();
}
