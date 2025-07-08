/*
 * BSD 2-Clause License
 *
 * Purpose: Declarations for non-local jump helpers.
 */
#ifndef SETJMP_H
#define SETJMP_H

#if defined(__has_include_next)
#  if __has_include_next(<setjmp.h>)
#    include_next <setjmp.h>
#  endif
#elif defined(__has_include)
#  if __has_include(<setjmp.h>)
#    include <setjmp.h>
#  endif
#endif

#ifdef setjmp
#undef setjmp
#endif
#ifdef longjmp
#undef longjmp
#endif

int setjmp(jmp_buf env);
void longjmp(jmp_buf env, int val);

#ifndef sigjmp_buf
typedef jmp_buf sigjmp_buf;
#endif

#ifndef sigsetjmp
int sigsetjmp(sigjmp_buf env, int save);
#endif
#ifndef siglongjmp
void siglongjmp(sigjmp_buf env, int val);
#endif

#endif /* SETJMP_H */
