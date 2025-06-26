/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms, with or without modification, are permitted provided that the copyright notice and this permission notice appear in all copies. This software is provided "as is" without warranty.
 *
 * Purpose: Implements the grp functions for vlibc. Provides wrappers and helpers used by the standard library.
 *
 * Copyright (c) 2025
 */

#include "grp.h"
#include "io.h"
#include "string.h"
#include "stdlib.h"
#include "env.h"
#include <fcntl.h>
#include <unistd.h>
#include "errno.h"
#include <sys/syscall.h>
#include "syscall.h"

#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)

static const char *group_path(void)
{
    const char *p = getenv("VLIBC_GROUP");
    if (p && *p)
        return p;
    return "/etc/group";
}

static struct group gr;
static char *members[64];
static char linebuf[256];

static struct group *parse_line(const char *line)
{
    strncpy(linebuf, line, sizeof(linebuf) - 1);
    linebuf[sizeof(linebuf) - 1] = '\0';

    char *save;
    gr.gr_name = strtok_r(linebuf, ":", &save);
    gr.gr_passwd = strtok_r(NULL, ":", &save);
    char *gid_s = strtok_r(NULL, ":", &save);
    char *mem_list = strtok_r(NULL, ":\n", &save);
    if (!gr.gr_name || !gr.gr_passwd || !gid_s || !mem_list)
        return NULL;
    gr.gr_gid = (gid_t)atoi(gid_s);

    char *save_mem;
    int i = 0;
    for (char *m = strtok_r(mem_list, ",", &save_mem); m &&
         i < (int)(sizeof(members)/sizeof(members[0]) - 1);
         m = strtok_r(NULL, ",", &save_mem)) {
        members[i++] = m;
    }
    members[i] = NULL;
    gr.gr_mem = members;
    return &gr;
}

static struct group *lookup(const char *name, gid_t gid, int by_name)
{
    int fd = open(group_path(), O_RDONLY, 0);
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
        struct group *g = parse_line(line);
        if (!g)
            continue;
        if (by_name) {
            if (strcmp(g->gr_name, name) == 0)
                return g;
        } else {
            if (g->gr_gid == gid)
                return g;
        }
    }
    return NULL;
}

struct group *getgrgid(gid_t gid)
{
    return lookup(NULL, gid, 0);
}

struct group *getgrnam(const char *name)
{
    return lookup(name, 0, 1);
}

extern void host_setgrent(void) __asm("setgrent");
extern struct group *host_getgrent(void) __asm("getgrent");
extern void host_endgrent(void) __asm("endgrent");

void setgrent(void)
{
    host_setgrent();
}

struct group *getgrent(void)
{
    return host_getgrent();
}

void endgrent(void)
{
    host_endgrent();
}

#else

extern struct group *host_getgrgid(gid_t) __asm("getgrgid");
extern struct group *host_getgrnam(const char *) __asm("getgrnam");

struct group *getgrgid(gid_t gid)
{
    return host_getgrgid(gid);
}

struct group *getgrnam(const char *name)
{
    return host_getgrnam(name);
}

static const char *group_path(void)
{
    const char *p = getenv("VLIBC_GROUP");
    if (p && *p)
        return p;
    return "/etc/group";
}

static struct group gr;
static char *members[64];
static char linebuf[256];
static char filebuf[4096];
static char *next_line;

static struct group *parse_line(const char *line)
{
    strncpy(linebuf, line, sizeof(linebuf) - 1);
    linebuf[sizeof(linebuf) - 1] = '\0';

    char *save;
    gr.gr_name = strtok_r(linebuf, ":", &save);
    gr.gr_passwd = strtok_r(NULL, ":", &save);
    char *gid_s = strtok_r(NULL, ":", &save);
    char *mem_list = strtok_r(NULL, ":\n", &save);
    if (!gr.gr_name || !gr.gr_passwd || !gid_s || !mem_list)
        return NULL;
    gr.gr_gid = (gid_t)atoi(gid_s);

    char *save_mem;
    int i = 0;
    for (char *m = strtok_r(mem_list, ",", &save_mem); m &&
         i < (int)(sizeof(members)/sizeof(members[0]) - 1);
         m = strtok_r(NULL, ",", &save_mem)) {
        members[i++] = m;
    }
    members[i] = NULL;
    gr.gr_mem = members;
    return &gr;
}

void setgrent(void)
{
    int fd = open(group_path(), O_RDONLY, 0);
    if (fd < 0) {
        next_line = NULL;
        return;
    }
    ssize_t n = read(fd, filebuf, sizeof(filebuf) - 1);
    close(fd);
    if (n <= 0) {
        next_line = NULL;
        return;
    }
    filebuf[n] = '\0';
    next_line = filebuf;
}

struct group *getgrent(void)
{
    if (!next_line)
        setgrent();
    if (!next_line)
        return NULL;
    char *line = strtok_r(next_line, "\n", &next_line);
    if (!line)
        return NULL;
    return parse_line(line);
}

void endgrent(void)
{
    next_line = NULL;
}

#endif

#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)

extern int host_getgrouplist(const char *, gid_t, gid_t *, int *)
    __asm("getgrouplist");
extern int host_initgroups(const char *, gid_t) __asm("initgroups");

int getgrouplist(const char *user, gid_t group, gid_t *groups, int *ngroups)
{
    return host_getgrouplist(user, group, groups, ngroups);
}

int initgroups(const char *user, gid_t group)
{
    return host_initgroups(user, group);
}

#else

static int do_setgroups(int n, const gid_t *groups)
{
#ifdef SYS_setgroups
    long ret = vlibc_syscall(SYS_setgroups, n, (long)groups, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
#else
    extern int host_setgroups(int, const gid_t *) __asm("setgroups");
    return host_setgroups(n, groups);
#endif
}

int getgrouplist(const char *user, gid_t group, gid_t *groups, int *ngroups)
{
    if (!user || !groups || !ngroups || *ngroups <= 0)
        return -1;

    int limit = *ngroups;
    int count = 0;
    groups[count++] = group;

    int fd = open(group_path(), O_RDONLY, 0);
    if (fd < 0)
        return -1;
    char buf[4096];
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (n <= 0)
        return -1;
    buf[n] = '\0';

    char *save_line;
    for (char *line = strtok_r(buf, "\n", &save_line); line;
         line = strtok_r(NULL, "\n", &save_line)) {
        struct group *g = parse_line(line);
        if (!g)
            continue;
        for (char **m = g->gr_mem; m && *m; m++) {
            if (strcmp(*m, user) == 0) {
                int dup = 0;
                for (int i = 0; i < count; i++) {
                    if (groups[i] == g->gr_gid) {
                        dup = 1;
                        break;
                    }
                }
                if (!dup) {
                    if (count < limit)
                        groups[count] = g->gr_gid;
                    count++;
                }
                break;
            }
        }
    }

    if (count > limit) {
        *ngroups = count;
        return -1;
    }
    *ngroups = count;
    return count;
}

int initgroups(const char *user, gid_t group)
{
    gid_t stack[32];
    gid_t *list = stack;
    int ng = (int)(sizeof(stack) / sizeof(stack[0]));
    int r = getgrouplist(user, group, list, &ng);
    if (r == -1) {
        list = malloc((size_t)ng * sizeof(gid_t));
        if (!list)
            return -1;
        if (getgrouplist(user, group, list, &ng) == -1) {
            free(list);
            return -1;
        }
    }
    int ret = do_setgroups(ng, list);
    if (list != stack)
        free(list);
    return ret;
}

#endif
