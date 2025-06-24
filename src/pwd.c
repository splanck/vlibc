/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms, with or without modification, are permitted provided that the copyright notice and this permission notice appear in all copies. This software is provided "as is" without warranty.
 *
 * Purpose: Implements the pwd functions for vlibc. Provides wrappers and helpers used by the standard library.
 *
 * Copyright (c) 2025
 */

#include "pwd.h"
#include "io.h"
#include "string.h"
#include "stdlib.h"
#include "env.h"
#include <fcntl.h>
#include <unistd.h>

#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)

static const char *passwd_path(void)
{
    const char *p = getenv("VLIBC_PASSWD");
    if (p && *p)
        return p;
    return "/etc/passwd";
}

static struct passwd pw;
static char linebuf[256];

static struct passwd *parse_line(const char *line)
{
    strncpy(linebuf, line, sizeof(linebuf) - 1);
    linebuf[sizeof(linebuf) - 1] = '\0';

    char *save;
    pw.pw_name = strtok_r(linebuf, ":", &save);
    pw.pw_passwd = strtok_r(NULL, ":", &save);
    char *uid_s = strtok_r(NULL, ":", &save);
    char *gid_s = strtok_r(NULL, ":", &save);
    pw.pw_gecos = strtok_r(NULL, ":", &save);
    pw.pw_dir = strtok_r(NULL, ":", &save);
    pw.pw_shell = strtok_r(NULL, ":\n", &save);
    if (!pw.pw_name || !pw.pw_passwd || !uid_s || !gid_s ||
        !pw.pw_gecos || !pw.pw_dir || !pw.pw_shell)
        return NULL;
    pw.pw_uid = (uid_t)atoi(uid_s);
    pw.pw_gid = (gid_t)atoi(gid_s);
    return &pw;
}

static struct passwd *lookup(const char *name, uid_t uid, int by_name)
{
    int fd = open(passwd_path(), O_RDONLY, 0);
    if (fd < 0)
        return NULL;
    char buf[4096];
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (n <= 0)
        return NULL;
    buf[n] = '\0';

    char *save_line;
    for (char *line = strtok_r(buf, "\n", &save_line); line;
         line = strtok_r(NULL, "\n", &save_line)) {
        struct passwd *p = parse_line(line);
        if (!p)
            continue;
        if (by_name) {
            if (strcmp(p->pw_name, name) == 0)
                return p;
        } else {
            if (p->pw_uid == uid)
                return p;
        }
    }
    return NULL;
}

struct passwd *getpwuid(uid_t uid)
{
    return lookup(NULL, uid, 0);
}

struct passwd *getpwnam(const char *name)
{
    return lookup(name, 0, 1);
}

#else

extern struct passwd *host_getpwuid(uid_t) __asm("getpwuid");
extern struct passwd *host_getpwnam(const char *) __asm("getpwnam");

struct passwd *getpwuid(uid_t uid)
{
    return host_getpwuid(uid);
}

struct passwd *getpwnam(const char *name)
{
    return host_getpwnam(name);
}

#endif
