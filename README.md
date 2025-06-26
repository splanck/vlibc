# vlibc

vlibc aims to provide a tiny C standard library for UNIX-like systems.
Its design keeps things simple and favors static linking so binaries
remain self-contained.

Designed as a minimal libc implementation for UNIX-like systems, vlibc
packs only the most essential runtime components needed by portable C
programs. Key features include:

- File I/O wrappers including directory-relative helpers like `openat`,
  `unlinkat`, `linkat`, `renameat` and `mknodat()`
- Permission checks with `access()` and `faccessat()`
- Process creation and control
- Execute programs from file descriptors with `fexecve()`
- Wait for children with `wait()` or `waitpid()` and decode results
  with macros from `<sys/wait.h>`
- Basic session and process-group APIs
- Threading primitives and simple read-write locks
- Thread-local storage helpers
- Non-blocking mutex acquisition with `pthread_mutex_trylock()`
- Timed waits on conditions with `pthread_cond_timedwait()`
- Wait for specific signals with `sigwait()` and `sigtimedwait()`
- Send queued signals with `sigqueue()`
- Set mutex types with `pthread_mutexattr_settype()`
- Counting semaphores with `sem_init()`/`sem_wait()`/`sem_post()`
- Named semaphores with `sem_open()`/`sem_close()`/`sem_unlink()`
- Thread barriers with `pthread_barrier_init()` and `pthread_barrier_wait()`
- Lightweight spin locks with `pthread_spin_lock()` and
  `pthread_spin_unlock()`
- Query the current thread ID with `pthread_self()` and compare IDs with `pthread_equal()`
- Spawn new threads with `pthread_create()` and wait with `pthread_join()`
  or detach with `pthread_detach()`
- Terminate threads with `pthread_exit()` or request cancellation via `pthread_cancel()`
- Networking sockets (`socket`, `bind`, `listen`, `accept`, `connect`,
  `getsockname`, `getpeername` and `shutdown`)
- Human-readable address errors with `gai_strerror()`
- Interface enumeration via `getifaddrs`
- Network byte order helpers with `htons`, `ntohs`, `htonl` and `ntohl`
- IPv4 text conversions with `inet_aton` and `inet_ntoa`
- Dynamic loading with `dlopen`, `dlsym`, `dlclose` and `dladdr`
- Environment variable handling
- Basic timezone handling with `tzset()`
- Minimal locale objects with `newlocale()` and `uselocale()`
- Format broken-down times with `asctime()` and thread-safe `asctime_r()`
- Host name queries and changes
- Login name retrieval with `getlogin()`
- Thread-safe user and group lookups with `getpwuid_r`, `getpwnam_r`,
  `getgrgid_r`, and `getgrnam_r`
- Enumerate all users or groups with `getpwent()` and `getgrent()`
- Secure password input with `getpass()`
- Password hashing with `crypt()`
- Syslog-style logging
- Simplified `err`/`warn` helpers for fatal and nonfatal messages
- Change root directories with `chroot()` when supported
- Change directories by descriptor with `fchdir()`
- Directory scanning helpers
- File tree traversal with `fts`
- Path expansion helpers with `glob()` and `wordexp()`
- Text encoding helpers with `vis()`, `nvis()` and `unvis()`
- Array sorting with `qsort`, `qsort_r` and `bsearch`
- Simple hash table via `hcreate`, `hdestroy` and `hsearch`
- Array resizing with `recallocarray()` which zeroes new memory
- Aligned allocations with `aligned_alloc()`
- Standard `assert` macro for runtime checks
- Extended math helpers including `hypot`, `round` and `trunc`
- Simple complex math with `cabs`, `carg`, `cexp`, `ccos` and `csin`
- Floating-point classification with `isnan()`, `isinf()` and `isfinite()`
- Extended character checks like `isprint`, `ispunct`, `iscntrl`,
  `isgraph` and `isblank`
- String-to-number conversions with `strtol`, `strtoul`, `strtoll`,
  `strtoull`, `strtof`, `strtod` and `strtold`
- Integer helpers `abs`, `labs`, `llabs` and division results with `div`,
  `ldiv` and `lldiv`
- POSIX-style 48-bit random numbers via the `rand48` family
- Deterministic uniform numbers with `arc4random_uniform()`
- Descriptor-based printing with `dprintf()`/`vdprintf()`
- Memory-backed streams with `open_memstream()`, `open_wmemstream()` and `fmemopen()`
- Replace an open stream with `freopen()`
- Large-file aware seeks
- Store and restore file positions with `fgetpos()` and `fsetpos()`
- Zero-copy file transfers with `sendfile()`
- Memory synchronization with `msync()`
- Lock pages with `mlock()`/`munlock()` and entire address spaces with
  `mlockall()`/`munlockall()` plus usage hints via `madvise()`
- POSIX shared memory objects with `shm_open()` and `shm_unlink()`
- POSIX message queues with `mq_open()`, `mq_send()` and `mq_receive()`
- System-V IPC primitives like `shmget()`, `semget()` and `msgget()`
- Advisory file locking with `flock()`
- FIFO creation with `mkfifo()` and `mkfifoat()`
- Device node creation with `mknod()` and `mknodat()`
- File space reservation with `posix_fallocate()`
- Access pattern hints with `posix_fadvise()`
- Memory usage hints with `posix_madvise()`
- Force file updates to disk with `fsync()` and `fdatasync()`
- Generic descriptor control with `ioctl()`
- Query terminal device paths with `ttyname()` and `ttyname_r()`
- Terminal attribute helpers `tcgetattr()`, `tcsetattr()`, `cfsetispeed()`,
  `cfgetispeed()`, `cfsetospeed()`, `cfgetospeed()`, `tcdrain()`, `tcflow()`, `tcflush()` and `tcsendbreak()`
- Pseudo-terminal helpers with `openpty()` and `forkpty()`
- Filesystem limits with `pathconf()` and `fpathconf()`
- Query configuration strings with `confstr()`
- Simple alarm timers with `alarm()`
- High resolution delays with `clock_nanosleep()` for relative or
  absolute sleeps
- Yield the processor with `sched_yield()` from `<sched.h>`
- Adjust and query nice values with `nice()`, `getpriority()` and
  `setpriority()` from `<sched.h>`
- POSIX interval timers with `timer_create` and `timer_settime()`.
  On NetBSD the native timer syscalls are used when present, with a
  kqueue fallback otherwise.
- Resource usage statistics with `getrusage()`
- Basic character set conversion with `iconv`
- Register quick-exit handlers with `at_quick_exit()` and trigger them via
  `quick_exit()`

**Note**: vlibc provides only a small subset of the standard C library. Some
functions depend on system calls tailored for Linux, though FreeBSD, OpenBSD
and NetBSD are all supported. Memory mapping routines fall back to
`MAP_ANON` when `MAP_ANONYMOUS` is unavailable. A few features may still rely
on platform-specific interfaces.

Build the static library with:

```sh
make
```

Install the headers and library system-wide with:

```sh
make install PREFIX=/usr/local
```

For packaging or staged installs, specify `DESTDIR`:

```sh
make install PREFIX=/usr/local DESTDIR=/tmp/vlibc-pkg
```

Headers are placed under `$PREFIX/include` and the static library under
`$PREFIX/lib` inside `DESTDIR` (or directly under the prefix if no
`DESTDIR` is provided).

## Quick Start

After building the library you can compile programs against `libvlibc.a`.
Below is a minimal hello world example:

```c
#include <stdio.h>

int main(void) {
    printf("hello, world\n");
    return 0;
}
```

Compile it with the vlibc headers and library:

```sh
cc -Iinclude hello.c libvlibc.a -o hello
```

Parsing strings is similarly straightforward using `sscanf`:

```c
int num;
char word[16];
sscanf("42 example", "%d %s", &num, word);
```

## String Helpers

vlibc ships common routines like `strdup`, `strndup`, `strlcpy`, and `strlcat`.
Search helpers `strstr`, `strrchr`, `memchr`, `memrchr`, and `memmem` locate substrings or
bytes. Tokenizers `strtok`, `strtok_r`, and the lightweight `strsep` help split
strings based on delimiters. Wide strings can be tokenized with `wcstok`.
Copy helpers `stpcpy` and `stpncpy` return the end pointer after copying.
`memccpy` stops when a byte is found while `mempcpy` returns the pointer to the
end of the copied region.

Launching a program in a new process with `posix_spawn` is similarly easy:

```c
pid_t pid;
char *args[] = {"/bin/echo", "spawn", NULL};
posix_spawn(&pid, "/bin/echo", NULL, NULL, args, environ);
waitpid(pid, NULL, 0);
```

`posix_spawn` now honors optional attributes and file actions so callers can
remap file descriptors or specify a signal mask for the new process.

For detailed documentation, see [docs/index.md](docs/index.md).
Suboption parsing with `getsubopt` is covered in
[docs/utilities.md](docs/utilities.md).

## Thread-Local Storage

Use `pthread_key_create` to allocate keys for storing per-thread data.
Values are set with `pthread_setspecific` and retrieved with
`pthread_getspecific`.  Keys can be removed using `pthread_key_delete`.
The `pthread_once` helper ensures an initializer runs only once across
all threads.

## Time Retrieval

Use `clock_gettime` for precise timestamps.

```c
struct timespec ts;
clock_gettime(CLOCK_MONOTONIC, &ts);
```

`CLOCK_REALTIME` returns the wall-clock time.
`clock_settime` adjusts it when privileges allow.
`clock_getres` queries the resolution of a given clock.
Use `difftime` to compute the difference between two `time_t` values.

## Time Formatting

`strftime` converts a `struct tm` to a human readable string. The
complementary `strptime` can parse the same fields back into a `struct tm`:

```c
struct tm tm = {0};
strptime("2023-05-06 07:08:09", "%Y-%m-%d %H:%M:%S", &tm);
```

Only `%Y`, `%m`, `%d`, `%H`, `%M`, and `%S` are understood.

## IPv6 Support

Networking helpers such as `inet_pton`, `inet_ntop`, `getaddrinfo` and
`getnameinfo` understand both IPv4 and IPv6 addresses. Use the standard
`AF_INET6` family to work with IPv6 sockets and address resolution.

## Socket Messaging

`sendmsg` and `recvmsg` send or receive arrays of buffers described by
`struct msghdr`.  This also enables passing ancillary data using
`struct cmsghdr`.

```c
struct iovec iov[2] = {
    { .iov_base = "he", .iov_len = 2 },
    { .iov_base = "llo", .iov_len = 3 }
};
struct msghdr msg = {0};
msg.msg_iov = iov;
msg.msg_iovlen = 2;
sendmsg(sock, &msg, 0);
```

### Interface Enumeration

`getifaddrs` returns a linked list describing each network interface. Iterate
through the list and free it with `freeifaddrs` when finished.

```c
struct ifaddrs *ifs;
if (getifaddrs(&ifs) == 0) {
    for (struct ifaddrs *i = ifs; i; i = i->ifa_next)
        printf("%s\n", i->ifa_name);
    freeifaddrs(ifs);
}
```

## Path Utilities

`basename` returns the final component of a path and `dirname` strips it
to yield the parent directory.

```c
char *b = basename("/usr/local/bin/tool");  // "tool"
char *d = dirname("/usr/local/bin/tool");   // "/usr/local/bin"
```

## Directory Scanning

`scandir` loads directory entries into a list. Pass `alphasort` as the
comparison function to sort results alphabetically.

```c
struct dirent **list;
int n = scandir(".", &list, NULL, alphasort);
for (int i = 0; i < n; i++) {
    printf("%s\n", list[i]->d_name);
    free(list[i]);
}
free(list);
```

`ftw` and `nftw` recursively walk a directory tree, invoking a callback
for each entry. Use `FTW_PHYS` with `nftw` to avoid following symbolic
links.
The `fts` interface provides an iterator-based walk returning each entry in turn.

## Display Width

`wcwidth` reports how many columns a single wide character occupies and
`wcswidth` sums the widths of up to `n` characters. This is useful when
laying out simple text UIs.

```c
int c = wcwidth(L'A');                // 1
int w = wcswidth(L"hello", 5);       // 5
```

Common helpers like `wcslen`, `wcscpy`, `wcsncpy`, `wcscmp`, `wcsncmp`, and
`wcsdup` mirror their narrow-string counterparts for manipulating wide
character strings. Memory routines `wmemcpy`, `wmemmove`, `wmemset` and
`wmemcmp` perform the same operations on arrays of `wchar_t`.

## Wide-Character I/O

Basic wide-character formatting and scanning are available through
`wprintf` and `wscanf`. Only the subset of conversion specifiers handled
by the regular `printf` implementation is currently recognized.

Individual wide characters can also be read and written using `fgetwc`
and `fputwc`:

```c
FILE *f = tmpfile();
fputwc(L'A', f);
rewind(f);
wchar_t wc = fgetwc(f);
// wc == L'A'
fclose(f);
```

## Platform Support

vlibc builds on Linux, FreeBSD, OpenBSD and NetBSD. NetBSD is now fully
supported alongside the other BSDs. Most functionality is portable across these
systems. On NetBSD native POSIX timers are used when available, while a few
other modules continue to rely on Linux-specific system calls.
Portable helpers like `sysconf()` and `getpagesize()` ease porting, but
non-Linux builds may require additional work. The `chroot()` wrapper is one
such case and returns `ENOSYS` when the underlying kernel lacks the system
call.

## Running Tests

Example tests live in the `tests/` directory. Run them with:

```sh
make test
```

This builds the test binary and exercises many of the library functions.

## License

This project is released under the terms of the
[BSD 2-Clause "Simplified" License](LICENSE).

