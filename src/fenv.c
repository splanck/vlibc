/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms, with
 * or without modification, are permitted provided that the copyright notice and
 * this permission notice appear in all copies. This software is provided "as is" without warranty.
 *
 * Purpose: Implements floating-point environment helpers for vlibc.
 */

#include "fenv.h"

#if defined(__has_builtin)
#  if __has_builtin(__builtin_feclearexcept)
#    define USE_BUILTIN_FECLEAREXCEPT 1
#  endif
#  if __has_builtin(__builtin_fesetround)
#    define USE_BUILTIN_FESETROUND 1
#  endif
#  if __has_builtin(__builtin_fegetround)
#    define USE_BUILTIN_FEGETROUND 1
#  endif
#  if __has_builtin(__builtin_feraiseexcept)
#    define USE_BUILTIN_FERAISEEXCEPT 1
#  endif
#  if __has_builtin(__builtin_fegetenv)
#    define USE_BUILTIN_FEGETENV 1
#  endif
#  if __has_builtin(__builtin_feholdexcept)
#    define USE_BUILTIN_FEHOLDEXCEPT 1
#  endif
#  if __has_builtin(__builtin_fesetenv)
#    define USE_BUILTIN_FESETENV 1
#  endif
#  if __has_builtin(__builtin_feupdateenv)
#    define USE_BUILTIN_FEUPDATEENV 1
#  endif
#  if __has_builtin(__builtin_fegetexceptflag)
#    define USE_BUILTIN_FEGETEXCEPTFLAG 1
#  endif
#  if __has_builtin(__builtin_fesetexceptflag)
#    define USE_BUILTIN_FESETEXCEPTFLAG 1
#  endif
#endif

#ifndef USE_BUILTIN_FECLEAREXCEPT
extern int host_feclearexcept(int) __asm__("feclearexcept");
#endif
#ifndef USE_BUILTIN_FESETROUND
extern int host_fesetround(int) __asm__("fesetround");
#endif
#ifndef USE_BUILTIN_FEGETROUND
extern int host_fegetround(void) __asm__("fegetround");
#endif
#ifndef USE_BUILTIN_FERAISEEXCEPT
extern int host_feraiseexcept(int) __asm__("feraiseexcept");
#endif
#ifndef USE_BUILTIN_FEGETENV
extern int host_fegetenv(fenv_t *) __asm__("fegetenv");
#endif
#ifndef USE_BUILTIN_FEHOLDEXCEPT
extern int host_feholdexcept(fenv_t *) __asm__("feholdexcept");
#endif
#ifndef USE_BUILTIN_FESETENV
extern int host_fesetenv(const fenv_t *) __asm__("fesetenv");
#endif
#ifndef USE_BUILTIN_FEUPDATEENV
extern int host_feupdateenv(const fenv_t *) __asm__("feupdateenv");
#endif
#ifndef USE_BUILTIN_FEGETEXCEPTFLAG
extern int host_fegetexceptflag(fexcept_t *, int) __asm__("fegetexceptflag");
#endif
#ifndef USE_BUILTIN_FESETEXCEPTFLAG
extern int host_fesetexceptflag(const fexcept_t *, int) __asm__("fesetexceptflag");
#endif

int feclearexcept(int excepts)
{
#ifdef USE_BUILTIN_FECLEAREXCEPT
    return __builtin_feclearexcept(excepts);
#else
    return host_feclearexcept(excepts);
#endif
}

int fegetexceptflag(fexcept_t *flagp, int excepts)
{
#ifdef USE_BUILTIN_FEGETEXCEPTFLAG
    return __builtin_fegetexceptflag(flagp, excepts);
#else
    return host_fegetexceptflag(flagp, excepts);
#endif
}

int fesetexceptflag(const fexcept_t *flagp, int excepts)
{
#ifdef USE_BUILTIN_FESETEXCEPTFLAG
    return __builtin_fesetexceptflag(flagp, excepts);
#else
    return host_fesetexceptflag(flagp, excepts);
#endif
}

int feraiseexcept(int excepts)
{
#ifdef USE_BUILTIN_FERAISEEXCEPT
    return __builtin_feraiseexcept(excepts);
#else
    return host_feraiseexcept(excepts);
#endif
}

int fetestexcept(int excepts)
{
    fexcept_t flag;
    fegetexceptflag(&flag, excepts);
    return flag & excepts;
}

int fegetround(void)
{
#ifdef USE_BUILTIN_FEGETROUND
    return __builtin_fegetround();
#else
    return host_fegetround();
#endif
}

int fesetround(int mode)
{
#ifdef USE_BUILTIN_FESETROUND
    return __builtin_fesetround(mode);
#else
    return host_fesetround(mode);
#endif
}

int fegetenv(fenv_t *envp)
{
#ifdef USE_BUILTIN_FEGETENV
    return __builtin_fegetenv(envp);
#else
    return host_fegetenv(envp);
#endif
}

int feholdexcept(fenv_t *envp)
{
#ifdef USE_BUILTIN_FEHOLDEXCEPT
    return __builtin_feholdexcept(envp);
#else
    return host_feholdexcept(envp);
#endif
}

int fesetenv(const fenv_t *envp)
{
#ifdef USE_BUILTIN_FESETENV
    return __builtin_fesetenv(envp);
#else
    return host_fesetenv(envp);
#endif
}

int feupdateenv(const fenv_t *envp)
{
#ifdef USE_BUILTIN_FEUPDATEENV
    return __builtin_feupdateenv(envp);
#else
    return host_feupdateenv(envp);
#endif
}

