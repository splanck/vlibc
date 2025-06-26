[← Back to index](index.md)

## File Permissions

Simple helpers adjust permissions and ownership:

```c
umask(022);
chmod("data.txt", 0644);
chown("data.txt", 1000, 1000);
fchmod(fd, 0644);
fchown(fd, 1000, 1000);
lchown("symlink", 1000, 1000);
mknod("/tmp/loop0", S_IFBLK | 0600, makedev(7, 0));
```
`mknod` creates special files like block or character devices when given a
device number.
`fchmodat` and `fchownat` perform the same operations relative to a directory
file descriptor.

## Filesystem *at Wrappers

These helpers mirror the basic file operations but operate relative to a
directory file descriptor.  `openat`, `unlinkat`, `mkdirat`, `symlinkat`,
`linkat`, `renameat`, `mknodat` and `fstatat` behave like their traditional
counterparts and accept `AT_FDCWD` to use the current working directory.

```c
int dfd = open("/tmp", O_DIRECTORY);
struct stat st;
int fd = openat(dfd, "demo.txt", O_WRONLY | O_CREAT, 0644);
fstatat(dfd, "demo.txt", &st, 0);
unlinkat(dfd, "demo.txt", 0);
```

On BSD systems these wrappers forward to the host C library.  Otherwise they
issue the corresponding syscalls when available.

## File Status

`stat`, `fstat`, and `lstat` from `sys/stat.h` query file metadata. The
`utime` and `utimes` helpers adjust the access and modification times of a
file:

```c
struct utimbuf t = { .actime = 1625097600, .modtime = 1625097600 };
utime("data.txt", &t);
```

## Filesystem Statistics

`statvfs` and `fstatvfs` from `sys/statvfs.h` report details about the
underlying filesystem such as block size and available space. On Linux
the vlibc wrappers issue the `statfs`/`fstatfs` syscalls and translate the
results. On BSD systems they simply call the host C library
implementations.

```c
struct statvfs sv;
if (statvfs("/", &sv) == 0) {
    printf("%lu blocks free\n", (unsigned long)sv.f_bfree);
}
```

`pathconf` and `fpathconf` query limits such as the maximum filename
length for a given directory. vlibc uses `sysconf` together with the
host `statvfs` implementation on BSD systems to answer these queries.

```c
long n = pathconf("/", _PC_NAME_MAX); /* typically 255 */
```

## Access Advisories

Applications may offer hints about expected file usage patterns with
`posix_fadvise`.  The call is a no-op on some platforms but allows the
kernel to optimize caching when supported.

```c
int fd = open("data.bin", O_RDONLY);
posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL);
```

Valid advice values include `POSIX_FADV_NORMAL`, `POSIX_FADV_RANDOM`,
`POSIX_FADV_SEQUENTIAL`, `POSIX_FADV_WILLNEED`, `POSIX_FADV_DONTNEED`
and `POSIX_FADV_NOREUSE`.

## Directory Iteration

Use `opendir`, `readdir`, and `closedir` from `dirent.h` to traverse
directories:

```c
DIR *d = opendir(".");
if (d) {
    struct dirent *e;
    while ((e = readdir(d))) {
        printf("%s\n", e->d_name);
    }
    closedir(d);
}
```

`scandir` collects entries into an array and can sort them with
`alphasort`:

```c
struct dirent **list;
int n = scandir("/tmp", &list, NULL, alphasort);
for (int i = 0; i < n; i++) {
    puts(list[i]->d_name);
    free(list[i]);
}
free(list);
```

`ftw` and `nftw` walk an entire directory tree, calling a function for
each entry. `nftw` provides additional flags such as `FTW_PHYS` to
avoid following symbolic links and `FTW_DEPTH` to visit directories
after their contents.

```c
static int cb(const char *path, const struct stat *st, int flag,
              struct FTW *info)
{
    (void)st; (void)info;
    printf("%d %s\n", flag, path);
    return 0;
}

nftw("/tmp", cb, 8, FTW_PHYS);
```

The `fts` interface offers an iterator that yields an `FTSENT`
for each visited file or directory:

```c
FTSENT *e;
char *const paths[] = {"/tmp", NULL};
FTS *f = fts_open(paths, FTS_PHYSICAL, NULL);
while ((e = fts_read(f)))
    printf("%s\n", e->fts_path);
fts_close(f);
```

## Path Canonicalization

`realpath` converts a pathname into an absolute canonical form. It
resolves `.` and `..` segments without consulting `/proc` so it works on
any POSIX system. Relative paths are expanded using the current working
directory.

```c
char buf[256];
realpath("tests/../", buf); // buf now holds the absolute path to the repository
```

## Path Utilities

`basename` returns the last component of a path while `dirname`
removes the trailing element. Both functions allocate a new string
for the result so the input remains untouched.

```c
char *b = basename("/usr/local/bin/tool");  // "tool"
char *d = dirname("/usr/local/bin/tool");   // "/usr/local/bin"
```

## Path Expansion

`glob` expands wildcard patterns like `*.c` into a list of matching
paths. It iterates through directories using `opendir` and
`readdir` and compares entries with an internal `fnmatch` implementation.
The matcher understands `*` and `?` wildcards as well as character classes
like `[a-z]`. Special characters may be escaped with a backslash unless
`FNM_NOESCAPE` is specified.

```c
glob_t g;
if (glob("src/*.c", 0, NULL, &g) == 0) {
    for (size_t i = 0; i < g.gl_pathc; i++)
        printf("%s\n", g.gl_pathv[i]);
    globfree(&g);
}
```

Results are sorted by default; pass `GLOB_NOSORT` to preserve the
filesystem order.

`wordexp` tokenizes a command-like string honoring quotes and escapes.
Each word undergoes `glob` expansion and leading `~` is replaced with the
user's home directory. The expanded words are returned in a `wordexp_t`
array which must be freed with `wordfree`.

```c
wordexp_t we;
if (wordexp("~/src/*.c", &we) == 0) {
    for (size_t i = 0; i < we.we_wordc; i++)
        printf("%s\n", we.we_wordv[i]);
    wordfree(&we);
}
```

