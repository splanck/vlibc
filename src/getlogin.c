/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms, with or without modification, are permitted provided that the copyright notice and this permission notice appear in all copies. This software is provided "as is" without warranty.
 *
 * Purpose: Implements the getlogin functions for vlibc. Provides wrappers and helpers used by the standard library.
 *
 * Copyright (c) 2025
 */

#include "unistd.h"
#include "pwd.h"
#include "string.h"

/* Retrieve the login name by calling getpwuid(getuid()) and cache
 * it in a thread-local buffer for reuse. */
char *getlogin(void)
{
    static __thread char name[64];
    if (name[0])
        return name;
    struct passwd *pw = getpwuid(getuid());
    if (!pw || !pw->pw_name)
        return NULL;
    strncpy(name, pw->pw_name, sizeof(name) - 1);
    name[sizeof(name) - 1] = '\0';
    return name;
}
