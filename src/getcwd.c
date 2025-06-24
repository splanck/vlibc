/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms, with or without modification, are permitted provided that the copyright notice and this permission notice appear in all copies. This software is provided "as is" without warranty.
 *
 * Purpose: Implements the getcwd functions for vlibc. Provides wrappers and helpers used by the standard library.
 *
 * Copyright (c) 2025
 */

#include "stdlib.h"

extern char *host_getcwd(char *buf, size_t size) __asm__("getcwd");

char *getcwd(char *buf, size_t size)
{
    return host_getcwd(buf, size);
}
