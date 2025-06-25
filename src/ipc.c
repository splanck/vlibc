/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms, with or without modification, are permitted provided that the copyright notice and this permission notice appear in all copies. This software is provided "as is" without warranty.
 *
 * Purpose: Implements IPC helpers for vlibc.
 */

#include "sys/ipc.h"
#include "sys/stat.h"
#include "errno.h"

key_t ftok(const char *path, int proj_id)
{
    struct stat st;
    if (stat(path, &st) < 0)
        return (key_t)-1;
    return (key_t)(((proj_id & 0xff) << 24) |
                   ((st.st_dev & 0xff) << 16) |
                   (st.st_ino & 0xffff));
}
