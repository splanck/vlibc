[← Back to index](index.md)

## User Database

`pwd.h` exposes minimal lookup helpers for entries in `/etc/passwd`.

```c
struct passwd {
    char *pw_name;
    char *pw_passwd;
    uid_t pw_uid;
    gid_t pw_gid;
    char *pw_gecos;
    char *pw_dir;
    char *pw_shell;
};

struct passwd *getpwuid(uid_t uid);
struct passwd *getpwnam(const char *name);
int getpwuid_r(uid_t uid, struct passwd *pwd, char *buf, size_t buflen,
               struct passwd **result);
int getpwnam_r(const char *name, struct passwd *pwd, char *buf, size_t buflen,
               struct passwd **result);
char *getlogin(void);
void setpwent(void);
struct passwd *getpwent(void);
void endpwent(void);
```

On BSD systems vlibc parses the file directly. The location can be
overridden via the `VLIBC_PASSWD` environment variable for testing.

`getpwuid_r()` and `getpwnam_r()` perform the same search but store
results in caller supplied memory so they are safe for concurrent use.

`getlogin()` obtains the user name for the current UID using
`getpwuid(getuid())`.  The resulting string is cached in thread-local
storage so repeated calls are inexpensive.

`setpwent()`, `getpwent()` and `endpwent()` iterate sequentially
through all passwd entries.  On BSD these wrappers call the host
libc while other systems parse the file directly.

## Group Database

`grp.h` provides minimal helpers for `/etc/group` entries.

```c
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
void setgrent(void);
struct group *getgrent(void);
void endgrent(void);
```

As with the password file, BSD platforms parse the group database directly.
The path can be overridden via the `VLIBC_GROUP` environment variable when
running tests. The `*_r` variants fill caller provided buffers and are
thread-safe.

`setgrent()`, `getgrent()` and `endgrent()` enumerate group entries in
order.  As with the passwd enumeration, BSD platforms use the host
libc while other systems parse `/etc/group` directly.

## Group Membership

Applications can inspect or modify a user's supplementary groups.

```c
int getgrouplist(const char *user, gid_t basegid,
                 gid_t *groups, int *ngroups);
int initgroups(const char *user, gid_t basegid);
```

`getgrouplist` fills `groups` with the IDs for `user`, starting with
`basegid`.  `*ngroups` specifies the array capacity and on return holds
the number of groups found.  If the buffer is too small the function
returns `-1` and updates `*ngroups` with the required size.

`initgroups` calls `setgroups()` to apply the list retrieved by
`getgrouplist`.  It typically requires appropriate privileges.

