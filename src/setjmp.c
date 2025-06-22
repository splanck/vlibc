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
    return _setjmp(env);
}

void longjmp(jmp_buf env, int val)
{
    _longjmp(env, val);
}
