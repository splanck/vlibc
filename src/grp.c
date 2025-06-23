#include "grp.h"
#include "io.h"
#include "string.h"
#include "stdlib.h"
#include "env.h"
#include <fcntl.h>
#include <unistd.h>

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

#endif
