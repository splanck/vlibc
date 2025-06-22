#include "stdlib.h"

#ifndef VLIBC_SHELL_PATH
#define VLIBC_SHELL_PATH "/bin/sh"
#endif

const char *vlibc_default_shell(void)
{
    const char *s = getenv("VLIBC_SHELL");
    if (s && *s)
        return s;
    return VLIBC_SHELL_PATH;
}
