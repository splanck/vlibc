/*
 * BSD 2-Clause License
 *
 * Purpose: Declarations for group database access.
 */
#ifndef GRP_H
#define GRP_H

#include <sys/types.h>
#include <stddef.h>

struct group {
    char *gr_name;
    char *gr_passwd;
    gid_t gr_gid;
    char **gr_mem;
};

struct group *getgrgid(gid_t gid);
struct group *getgrnam(const char *name);
int getgrgid_r(gid_t gid, struct group *grp, char *buf, size_t buflen,
               struct group **result);
int getgrnam_r(const char *name, struct group *grp, char *buf, size_t buflen,
               struct group **result);

#endif /* GRP_H */
