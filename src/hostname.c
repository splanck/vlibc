/*
 * BSD 2-Clause "Simplified" License
 *
 * Copyright (c) 2025, vlibc authors
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Purpose: Implements the hostname functions for vlibc.
 */

#include "env.h"
#include "errno.h"
#include <sys/syscall.h>
#include <unistd.h>
#include "syscall.h"

/* gethostname() - retrieve current host name */
int gethostname(char *name, size_t len)
{
#ifdef SYS_gethostname
    long ret = vlibc_syscall(SYS_gethostname, (long)name, len, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
#else
    extern int host_gethostname(char *, size_t) __asm__("gethostname");
    return host_gethostname(name, len);
#endif
}

/* sethostname() - set the system host name */
int sethostname(const char *name, size_t len)
{
#ifdef SYS_sethostname
    long ret = vlibc_syscall(SYS_sethostname, (long)name, len, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
#else
    extern int host_sethostname(const char *, size_t) __asm__("sethostname");
    return host_sethostname(name, len);
#endif
}
