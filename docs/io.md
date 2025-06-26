[← Back to index](index.md)

## Basic File I/O

Thin wrappers around the kernel's file APIs live in `io.h`. Functions
like `open`, `read`, `write`, `close`, `unlink`, `rename`, `symlink`,
`mkdir`, `rmdir`, `chdir`, and the file size controls `truncate`,
`ftruncate` and `posix_fallocate` simply pass their arguments to the
corresponding syscalls or suitable fallbacks.
`access` and `faccessat` query permissions on files without opening
them.
Vector I/O through `readv` and `writev` is available to gather or scatter
multiple buffers in a single call.  The offset aware variants `preadv`
and `pwritev` operate on multiple buffers at a specific file position.
Zero-copy transfers are possible with
`sendfile`, which uses the BSD system call when available and otherwise
falls back to copying via `read` and `write`.

```c
int fd = open("log.txt", O_WRONLY | O_CREAT, 0644);
if (fd >= 0) {
    write(fd, "hello\n", 6);
    close(fd);
}
link("log.txt", "log.bak");
symlink("log.txt", "log.lnk");
char buf[32];
ssize_t n = readlink("log.lnk", buf, sizeof(buf) - 1);
if (n >= 0) {
    buf[n] = '\0';
}
```

Retrieve the current directory with `getcwd` and switch directories
with `chdir`:

```c
char path[128];
if (getcwd(path, sizeof(path)))
    printf("cwd: %s\n", path);
chdir("/");
```

`chroot` confines the process to a new root directory. The wrapper simply
invokes the `chroot(2)` system call when available and fails with `ENOSYS`
on platforms lacking the call. For spawning child processes see
[process.md](process.md).

Additional helpers inspect and reset a stream's state. `feof(stream)`
returns non-zero once the end of input has been reached while
`ferror(stream)` indicates an I/O error.  Call `clearerr(stream)` to reset
both flags.  `fileno(stream)` exposes the underlying descriptor number and
`ungetc(c, stream)` pushes a single byte back so the next `fgetc` returns
it.

```c
FILE *f = fopen("data.txt", "r");
char buf[16];
while (f && !feof(f)) {
    size_t n = fread(buf, 1, sizeof(buf), f);
    if (ferror(f)) {
        perror("read error");
        break;
    }
    /* process n bytes */
}
clearerr(f);
fclose(f);
```

## File Descriptor Helpers

Low-level descriptor routines perform simple tasks such as repositioning a file,
reading or writing at a specific offset, or duplicating handles.

```c
off_t pos = lseek(fd, 0, SEEK_SET);
ssize_t n = pread(fd, buf, 16, 4);
pwrite(fd, buf, n, 32);
struct iovec v[2] = { {buf, 4}, {buf + 4, 4} };
preadv(fd, v, 2, 0);
pwritev(fd, v, 2, 8);
int duped = dup(fd);
int pipefd[2];
pipe(pipefd);
```

Flush pending writes with `fsync(fd)` or the data-only variant
`fdatasync(fd)`. Both return 0 on success.  `sync()` forces the kernel to
write all buffered filesystem data to disk.

Use `isatty(fd)` to query whether a descriptor refers to a terminal.

## File Control

`fcntl` adjusts descriptor flags such as `FD_CLOEXEC` or toggles
non-blocking mode:

```c
int fl = fcntl(fd, F_GETFL);
fcntl(fd, F_SETFL, fl | O_NONBLOCK);
fcntl(fd, F_SETFD, FD_CLOEXEC);
```

`ioctl` performs device-specific operations on a descriptor. Requests are
platform-specific:

```c
struct winsize ws;
ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
```

## File Locking

Use `flock` to coordinate access to files between multiple processes. See
[process.md](process.md) for process creation utilities.
Shared locks (`LOCK_SH`) allow concurrent readers while an exclusive lock
(`LOCK_EX`) grants write access. Combine `LOCK_NB` to return immediately if
the lock cannot be obtained.

```c
int fd = open("data.txt", O_RDWR);
flock(fd, LOCK_EX);
/* operate on the file */
flock(fd, LOCK_UN);
```

## Terminal Attributes

`tcgetattr` reads the settings for a terminal and `tcsetattr` modifies
them. `cfmakeraw` adjusts a `struct termios` to raw mode for interactive
programs:

```c
struct termios t;
if (tcgetattr(STDIN_FILENO, &t) == 0) {
    cfmakeraw(&t);
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}
```

Additional helpers wait for output or control data queues. `tcdrain`
blocks until all written data has been transmitted. `tcflush` can
discard queued input or output, while `tcflow` suspends or resumes
transmission. `tcsendbreak` sends a break condition for the specified
duration.

## Pseudo-terminals

`openpty` creates a master/slave pair of descriptors. Optional `termios` and
`winsize` parameters configure the initial terminal settings. `forkpty` combines
`openpty` with `fork` and attaches the slave as the controlling terminal of the
child.

```c
int mfd;
pid_t pid = forkpty(&mfd, NULL, NULL, NULL);
if (pid == 0) {
    execlp("/bin/sh", "sh", NULL);
}
write(mfd, "echo hi\n", 8);
```

## Secure Password Input

`getpass` reads a line from `/dev/tty` with echo disabled. The typed
password is returned in a static buffer so callers should copy it if it
needs to be preserved.

```c
char *pw = getpass("Password: ");
```

## Password Hashing

`crypt` supports multiple hashing schemes. The classic DES algorithm is
always available.  Prefixes select stronger hashes:

- `$1$` – BSD MD5
- `$5$` – SHA-256
- `$6$` – SHA-512

If an unknown prefix is supplied on BSD platforms the call is delegated
to the host `crypt` implementation.  The two-character DES format
remains portable across all targets.

## Standard Streams

`stdin`, `stdout`, and `stderr` are lightweight streams wrapping file
descriptors 0, 1 and 2. They can be used with the provided `fread`,
`fwrite`, `fseek`, `ftell`, `fseeko`, `ftello`, `fgetpos`, `fsetpos`, `rewind`, `fgetc`, `fputc`, `ungetc`, `fgets`,
`fputs`, `sprintf`, `snprintf`, `asprintf`, `vasprintf`, `vsprintf`,
`vsnprintf`, `fprintf`, `vfprintf`, `dprintf`, `vdprintf`, `printf`, `vprintf`, `vsscanf`, `vfscanf`, `vscanf`,
`sscanf`, `fscanf`, `scanf`, `getline`, and `getdelim` helpers.  Query
stream state with `feof`, `ferror`, and `clearerr`, obtain the descriptor
number via `fileno`, or wrap an existing descriptor with `fdopen`.
`fflush(stream)` succeeds
and invokes `fsync` on the descriptor when one is present.

`asprintf` and `vasprintf` return a newly allocated string sized by
`vsnprintf`.

`getline` and `getdelim` grow the supplied buffer automatically while reading.

Use `open_memstream` to capture output into a dynamically growing buffer or
`fmemopen` to read and write to an existing memory region.

Streams may be given a custom buffer with `setvbuf` or the simpler
`setbuf`. When buffered, I/O operates on that memory until it is filled
or explicitly flushed.

`freopen` can replace the file associated with an existing stream so the same
`FILE` handle refers to a new path. This is handy for redirecting a standard
stream:

```c
FILE *f = fopen("swap.txt", "w");
fputs("hello", f);
freopen("swap.txt", "r", f);
char buf[16];
fgets(buf, sizeof(buf), f);
```

```c
int a;
unsigned b;
char word[16];
sscanf("1 2 hi", "%d %u %s", &a, &b, word);
unsigned hex;
unsigned oct;
sscanf("ff 12", "%x %o", &hex, &oct);
```

Using the `vscanf` family directly allows forwarding a `va_list`:

```c
void parse_numbers(const char *buf, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsscanf(buf, fmt, ap);
    va_end(ap);
}
```

## Temporary Files

`mkstemp` replaces the trailing `XXXXXX` in a template with random
characters and opens the resulting file. `mkdtemp` performs the same
replacement but creates a directory instead. `tmpfile` returns a stream
backed by an anonymous temporary file that is unlinked immediately.

```c
char path[] = "/tmp/exampleXXXXXX";
int fd = mkstemp(path);
FILE *anon = tmpfile();
char dir[] = "/tmp/exampledirXXXXXX";
mkdtemp(dir);
mkfifo("/tmp/myfifo", 0600);
```

`mkfifo` creates a FIFO special file. The `mkfifoat` variant allows
specifying a directory file descriptor in addition to the path.

