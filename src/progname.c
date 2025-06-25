/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms,
 * with or without modification, are permitted provided that the copyright
 * notice and this permission notice appear in all copies. This software is
 * provided "as is" without warranty.
 *
 * Purpose: Implements program name helpers for vlibc.
 */

#include "string.h"
#include "util.h"

static const char *progname = "";

const char *getprogname(void)
{
    return progname;
}

void setprogname(const char *name)
{
    if (!name) {
        progname = "";
        return;
    }
    const char *base = strrchr(name, '/');
    progname = base ? base + 1 : name;
}
