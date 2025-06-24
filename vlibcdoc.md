# vlibc Documentation

This document outlines the architecture, planned modules, and API design for **vlibc**. The goal is to provide a clear overview for contributors and users who wish to understand how the library is organized. For usage examples see the [README](README.md).

## Table of Contents

1. [Overview](#overview)
2. [Architecture](#architecture)
3. [Provided Headers](#provided-headers)
4. [Memory Management](#memory-management)
5. [Memory Mapping](#memory-mapping)
6. [String Handling](#string-handling)
7. [Character Classification](#character-classification)
8. [Option Parsing](#option-parsing)
9. [Random Numbers](#random-numbers)
10. [Sorting Helpers](#sorting-helpers)
11. [Regular Expressions](#regular-expressions)
12. [Math Functions](#math-functions)
13. [Process Control](#process-control)
14. [Error Reporting](#error-reporting)
15. [Errno Access](#errno-access)
16. [Threading](#threading)
17. [Dynamic Loading](#dynamic-loading)
18. [Environment Variables](#environment-variables)
19. [System Information](#system-information)
20. [Basic File I/O](#basic-file-io)
21. [File Descriptor Helpers](#file-descriptor-helpers)
22. [File Control](#file-control)
23. [File Locking](#file-locking)
24. [Terminal Attributes](#terminal-attributes)
25. [Secure Password Input](#secure-password-input)
26. [Standard Streams](#standard-streams)
27. [Temporary Files](#temporary-files)
28. [Networking](#networking)
29. [I/O Multiplexing](#io-multiplexing)
30. [File Permissions](#file-permissions)
31. [Filesystem *at Wrappers](#filesystem-at-wrappers)
32. [File Status](#file-status)
33. [Directory Iteration](#directory-iteration)
34. [Path Canonicalization](#path-canonicalization)
35. [Path Utilities](#path-utilities)
36. [User Database](#user-database)
37. [Group Database](#group-database)
38. [Time Formatting](#time-formatting)
39. [Locale Support](#locale-support)
40. [Time Retrieval](#time-retrieval)
41. [Sleep Functions](#sleep-functions)
42. [Interval Timers](#interval-timers)
43. [Raw System Calls](#raw-system-calls)
44. [Non-local Jumps](#non-local-jumps)
45. [Limitations](#limitations)
46. [Conclusion](#conclusion)
47. [Logging](#logging)
48. [Path Expansion](#path-expansion)
49. [Filesystem Statistics](#filesystem-statistics)
50. [Resource Limits](#resource-limits)

## Overview

vlibc is a minimal libc replacement targeted at UNIX-like systems. It provides a
small subset of the standard C library with an emphasis on simplicity and
predictability. The project is distributed under the terms of the GNU General
Public License v3.0.

### Goals
- Keep the code base small and auditable.
- Supply only the essential C runtime facilities.
- Offer consistent behavior for statically linked or embedded programs.

### Target Platforms
- Linux (x86_64, aarch64, armv7) is currently the primary platform.
 - *BSD variants are partially supported and patches are welcome.
- Other POSIX systems should work with minimal porting effort.
- Lightweight containers or small research kernels implementing POSIX system
  calls.

### Guiding Principles
- Minimalism: implement just enough functionality for common use.
- Readability over clever micro-optimizations.
- Prefer direct system calls and avoid heavy runtime features.
- Encourage static linking to reduce external dependencies.

### Motivation and Use Cases
vlibc was created for developers who need a tiny C runtime for experiments,
educational operating systems, or compact container images. It serves as a
foundation for projects that require libc functionality but do not want the
complexity of larger implementations.

### Differences from Existing Solutions
While glibc and musl provide comprehensive POSIX support, vlibc focuses on a
much smaller API surface. It is **not** a drop-in replacement for the full
standard library. Instead, vlibc offers a minimal set of wrappers and utilities
that can be easily inspected or extended.

### Building the Library

```sh
make
```

This produces `libvlibc.a` in the repository root. Install the headers and
library with:

```sh
make install PREFIX=/usr/local
```

### Running Tests

```sh
make test
```

This builds `tests/run_tests` and executes the suite.

## Architecture

vlibc is structured around small, easily auditable modules. Each module exposes a
minimal API and relies on direct system calls whenever possible. Source files
live in `src/`, public headers in `include/`, and tests reside in `tests/`.

### Planned Modules
- **startup**: Program entry, stack setup and initialization routines.
- **memory**: Basic heap management using `brk`/`sbrk` with a fallback to
  `mmap`/`munmap` when those calls are unavailable.
- **io**: Simple wrappers around read/write system calls for file descriptors.
- **string**: Common string operations such as length checks and copying.
- **process**: Functions for spawning and waiting on child processes.
- **math**: Elementary math routines like `sin` and `sqrt` plus hyperbolic
  helpers and `atan2`.

### API Design
vlibc strives for a consistent API surface inspired by the C standard library.
Functions are prefixed with `v` (e.g., `vmalloc`) to differentiate them from
their libc counterparts. Where appropriate, functions mirror POSIX signatures so
users can easily migrate existing code.

### Naming Conventions
- `v` prefix for all exported functions.
- Lowercase with underscores for function names and variables.
- Keep parameter lists short and intuitive.

### Initialization
Initialization code lives in the **startup** module and performs:

1. Setting up the runtime stack and registers.
2. Parsing arguments and environment variables.
3. Calling `v_main()` as the program entry point.
4. Providing a minimal exit routine to flush data and terminate.

Programs using vlibc define `v_main(argc, argv, envp)` to keep the startup
routine simple and avoid complex features like C++ static constructors.

## Provided Headers

vlibc installs a small set of public headers for application use:

```c
ctype.h      - character classification helpers
dirent.h     - directory iteration
dlfcn.h      - runtime loading of shared libraries
env.h        - environment variable access
errno.h      - standard error codes
getopt.h     - option parsing
io.h         - unbuffered I/O primitives
locale.h     - locale helpers
math.h       - basic math routines
memory.h     - heap allocation
assert.h     - runtime assertion checks
netdb.h      - address resolution helpers
arpa/inet.h  - IPv4/IPv6 presentation conversion helpers
ftw.h        - directory tree traversal helpers
poll.h       - I/O multiplexing helpers
signal.h    - signal handling helpers
process.h    - process creation and control
pthread.h    - minimal threading support
setjmp.h     - non-local jump helpers
stdio.h      - simple stream I/O
stdlib.h     - basic utilities
string.h     - string manipulation
iconv.h      - character set conversion helpers
regex.h     - simple regular expression matching
termios.h   - terminal attribute helpers
unistd.h    - POSIX I/O and process helpers
sys/file.h   - file permission helpers
sys/mman.h   - memory mapping helpers
sys/select.h - fd_set macros and select wrapper
sys/socket.h - networking wrappers
sys/stat.h   - file status functions
sys/statvfs.h - filesystem statistics
syscall.h    - raw syscall interface
time.h       - time related helpers
vlibc.h      - library initialization
wchar.h      - wide character helpers
wctype.h     - wide character classification helpers
```


## Memory Management

The **memory** module provides a very small heap allocator implemented in
`memory.c`. When available it uses the `sbrk` system call to extend the heap.
On systems without `sbrk` the allocator falls back to `mmap`/`munmap` for
obtaining memory from the kernel. The implementation deliberately keeps things
simple. Each allocation stores a small header so the most recent block can be
released on `free()`. Memory for older blocks is not recycled when using
`sbrk`, keeping the code easy to audit at the cost of efficiency. When built
with the `mmap` backend each `free` call unmaps the region entirely.

### API

```c
void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
void *reallocarray(void *ptr, size_t nmemb, size_t size);
int posix_memalign(void **memptr, size_t alignment, size_t size);
```

### Behavior and Caveats

- When compiled with `HAVE_SBRK`, `malloc` allocates memory linearly using
  `sbrk` and returns `NULL` on failure. `free` only releases memory if called on
  the most recently allocated block.
- When `sbrk` is unavailable `malloc` obtains pages with `mmap` and `free`
  unmaps them with `munmap`.
- `calloc` calls `malloc` and zeroes the allocated block.
- `realloc` always allocates a new block and copies up to `size` bytes from the
  old pointer if one was provided.
- `reallocarray` multiplies `nmemb` and `size` with overflow checks, returning
  `NULL` and setting `errno` to `ENOMEM` on overflow.
- `posix_memalign` stores the allocated pointer in `*memptr` with the requested
  alignment. It returns `0` on success, `EINVAL` if the alignment is not a power
  of two or not a multiple of `sizeof(void *)`, or `ENOMEM` when the allocation
  fails.

```c
void *p;
if (posix_memalign(&p, 64, 128) == 0) {
    /* use memory */
    free(p);
}
```

Because memory is only reclaimed for the most recent block, applications that
allocate many objects may still eventually exhaust the heap. These routines are
sufficient for
small examples but should not be considered production quality.

## Memory Mapping

The `sys/mman.h` header exposes wrappers for interacting with the kernel's
memory mapping facilities. Available functions are:

```c
void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int munmap(void *addr, size_t length);
int mprotect(void *addr, size_t length, int prot);
int msync(void *addr, size_t length, int flags);
```

`mmap` creates new mappings, `munmap` releases them and `mprotect` changes
their access protections.  `msync` flushes modified pages back to their
underlying file.  When the raw syscall is unavailable the BSD wrapper is
used instead, so some platforms may ignore unsupported flags.

## String Handling

The **string** module provides fundamental operations needed by most C programs:

- `vstrlen`, `vstrcpy`, `vstrncmp`, `strnlen`, `strcat`, `strncat`, `strlcpy` and `strlcat` equivalents.
- `strdup` and `strndup` helpers allocate new copies of strings.
- Search helpers `strstr`, `strrchr`, and `memchr` for locating substrings or bytes.
- Prefix scanners `strspn` and `strcspn` along with `strpbrk` for finding any character from a set.
- Case-insensitive comparisons `strcasecmp` and `strncasecmp`.
- Basic collation helpers `strcoll` and `strxfrm` act on ASCII strings. On
  BSD systems they defer to the host implementations when the active locale is
  not `"C"` or `"POSIX"`.
- Conventional memory routines (`memcpy`, `memmove`, `memset`, `memcmp`) map to
  the internal `v` implementations.
- The low-level memory helpers `vmemcpy`, `vmemmove`, `vmemset`, and `vmemcmp` operate on raw byte buffers. `vmemcpy` copies bytes from a source to a destination, `vmemmove` handles overlaps safely, `vmemset` fills a region with a byte value, and `vmemcmp` compares two buffers. The standard `memcpy`, `memmove`, `memset`, and `memcmp` functions simply call these implementations.
- Basic locale handling reads the `LC_ALL` and `LANG` environment variables.
  `setlocale` defaults to those values and, on BSD systems, falls back to the
  host `setlocale(3)` when a locale other than `"C"` or `"POSIX"` is
  requested. `localeconv` exposes formatting data. All strings are treated as
  byte sequences.
- Utility functions for tokenizing and simple formatting.
- `printf` style routines understand `%d`, `%u`, `%s`, `%x`, `%X`, `%o`, `%p`,
  and `%c` with basic field width and precision handling.
- `scanf` style routines parse `%d`, `%u`, `%x`, `%o`, `%s`, and floating
  point formats such as `%f`, `%lf`, and `%g`.
- `strtok` and `strtok_r` split a string into tokens based on a set of
  delimiter characters. `strtok` stores its parsing state in static
  memory and is not thread-safe. `strtok_r` lets the caller maintain the
  context and is safe for concurrent use.
  - Simple number conversion helpers `atoi`, `strtol`, `strtoul`, `strtoll`,
    `strtoull`, `strtod`, and `atof`.

### Example

```c
const char *text = "hello world";
size_t first_word = strcspn(text, " ");
char *vowel = strpbrk(text, "aeiou");
size_t prefix = strspn("abc123", "abc");
```

Basic time formatting is available via `strftime` and the matching
`strptime` parser. `strftime` handles common conversions like
`%Y`, `%m`, `%d`, `%H`, `%M`, `%S`, `%a`, `%b`, `%Z`, `%z`, and weekday
numbers (`%w`/`%u`). The parser continues to accept the numeric
fields (`%Y`, `%m`, `%d`, `%H`, `%M`, `%S`). The output uses the current
locale. Non-`"C"` locales work when the host `setlocale(3)` accepts them
(primarily on BSD systems).

The library also includes simple conversion routines `gmtime`, `localtime`,
`mktime`, and `ctime`. They convert between `time_t` and `struct tm` or
produce a readable string. `localtime` ignores the system timezone so the
result is identical to `gmtime`.

The goal is to offer just enough functionality for common tasks without the complexity of full locale-aware libraries.

### Wide Character Conversion

`mbtowc` converts a multibyte sequence to a single `wchar_t` and `wctomb`
performs the opposite conversion.  `mbrtowc`/`wcrtomb` are stateful
variants used by `mbstowcs` and `wcstombs` for converting entire
strings.  These helpers handle ASCII directly and fall back to the host
C library when encountering non-ASCII data.  `wcslen` returns the length
of a wide string excluding the terminator.

`wcwidth` reports the number of columns needed to display a single wide
character while `wcswidth` sums the widths of up to `n` characters. ASCII
codepoints have the expected width and on BSD systems non-ASCII values
are delegated to the host implementation.

### Example

```c
int c = wcwidth(L'A');               // 1
int w = wcswidth(L"hello", 5);      // 5
```

### Character Set Conversion

`iconv_open` returns a descriptor for translating between character
sets.  vlibc understands only conversions between `"ASCII"` and
`"UTF-8"`.  The `iconv` function copies bytes from the input buffer to
the output buffer and fails with `EILSEQ` on bytes that cannot be
represented.  On BSD systems other conversions are delegated to the
host `iconv` implementation when present.

## Character Classification

Character checks live in [include/ctype.h](include/ctype.h).  The macros
operate on a small table of 128 entries, so only the standard ASCII
characters are recognized.

Wide-character classification is provided in [include/wctype.h](include/wctype.h).
The functions there test a few basic Unicode ranges and fall back to the host
implementation on BSD systems when available.

- `isalpha(c)`  – non-zero for letters `A`–`Z` or `a`–`z`.
- `isdigit(c)`  – non-zero for decimal digits `0`–`9`.
- `isalnum(c)`  – true when `isalpha(c)` or `isdigit(c)` is true.
- `isspace(c)`  – tests for whitespace characters such as space or tab.
- `isupper(c)`  – non-zero for uppercase letters.
- `islower(c)`  – non-zero for lowercase letters.
- `isxdigit(c)` – true for hexadecimal digits.
- `tolower(c)`  – converts an uppercase letter to lowercase.
- `toupper(c)`  – converts a lowercase letter to uppercase.

Values outside the ASCII range always fail the classification tests and are
returned unchanged by `tolower` and `toupper`.

## Option Parsing

Command-line arguments can be processed with `getopt`, `getopt_long` or
`getopt_long_only`. The former handles short options while the latter
two accept an array of `struct option` describing long names. The
`getopt_long_only` variant also recognizes long options when they are
prefixed with a single dash. Both long parsing functions return the
option's value field or set a flag when supplied in the table.

## Random Numbers

vlibc provides a minimal pseudo-random number generator implemented as a
linear congruential generator.

```c
int rand(void);
void srand(unsigned seed);
unsigned int arc4random(void);
void arc4random_buf(void *buf, size_t len);
int rand_r(unsigned *state);
```

Calling `srand()` initializes the internal state. Reusing the same seed
produces the identical sequence of numbers, each in the range `0` to
`32767`.

`arc4random()` returns a 32-bit value sourced from the operating system's
random generator. `arc4random_buf()` fills an arbitrary buffer with secure
random bytes. The `rand_r()` variant operates like `rand()` but stores its
state in a user-provided variable so it can be used in threaded code.

## Sorting Helpers

`qsort` sorts an array in place using a user-supplied comparison
function while `bsearch` performs binary search on a sorted array.

```c
int values[] = {4, 2, 7};
qsort(values, 3, sizeof(int), cmp_int);
int key = 7;
int *found = bsearch(&key, values, 3, sizeof(int), cmp_int);
```

## Regular Expressions

The `regex` module offers a lightweight matcher covering a small
subset of POSIX expressions.  Patterns are compiled with `regcomp`
and executed using `regexec`.

```c
regex_t re;
regcomp(&re, "h.*o", 0);
if (regexec(&re, "hello", 0, NULL, 0) == 0) {
    /* matched */
}
regfree(&re);
```

Only simple features are implemented: `.` matches any character,
`*`, `+` and `?` provide repetition, `[]` defines character classes
and `^`/`$` anchor to the start or end of the string. Parentheses
create capture groups which can be referenced later in the pattern
using backreferences like `\1` and `\2`. Groups may not be used
with repetition operators.

Example with a backreference:

```c
regex_t r;
regcomp(&r, "(foo)bar\\1", 0);
if (regexec(&r, "foobarfoo", 0, NULL, 0) == 0) {
    /* matches */
}
regfree(&r);
```

## Math Functions

`sin`, `cos`, `tan`, `sqrt`, `pow`, `log`, `log2`, `log10`, `exp`, `ldexp`,
`floor`, `ceil`, `fmod`, `fabs`, `fabsf`, `fmin`, `fmax`, `copysign`,
`atan2`, `sinh`, `cosh`, and `tanh` are provided in `math.h`. These use simple
series approximations
and are suitable for basic calculations but may lack high precision.

## Process Control

Process-related functionality resides in the **process** module. It provides
minimal wrappers for creating and managing processes, querying process IDs,
and installing signal handlers.  The companion `signal.h` header offers
`sigaction`, `sigprocmask` and helpers for manipulating signal sets:

```c
pid_t fork(void);
int execve(const char *pathname, char *const argv[], char *const envp[]);
int execvp(const char *file, char *const argv[]);
int execv(const char *path, char *const argv[]);
int execl(const char *path, const char *arg, ...);
int execlp(const char *file, const char *arg, ...);
int execle(const char *path, const char *arg, ...);
int posix_spawn(pid_t *pid, const char *path,
                const posix_spawn_file_actions_t *file_actions,
                const posix_spawnattr_t *attrp,
                char *const argv[], char *const envp[]);
int posix_spawnp(pid_t *pid, const char *file,
                 const posix_spawn_file_actions_t *file_actions,
                 const posix_spawnattr_t *attrp,
                 char *const argv[], char *const envp[]);
pid_t wait(int *status);
pid_t waitpid(pid_t pid, int *status, int options);
int kill(pid_t pid, int sig);
pid_t getpid(void);
pid_t getppid(void);
int setpgid(pid_t pid, pid_t pgid);
pid_t getpgid(pid_t pid);
pid_t setsid(void);
pid_t getsid(pid_t pid);
sighandler_t signal(int signum, sighandler_t handler);
uid_t getuid(void);
uid_t geteuid(void);
gid_t getgid(void);
gid_t getegid(void);
int setuid(uid_t uid);
int seteuid(uid_t euid);
int setgid(gid_t gid);
int setegid(gid_t egid);
int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
int sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
int system(const char *command);
int daemon(int nochdir, int noclose);
int atexit(void (*fn)(void));
void abort(void);
void exit(int status);
```

These wrappers retrieve and manipulate process information. `getuid`,
`geteuid`, `getgid`, and `getegid` return the real and effective user and
group IDs. `setuid`, `seteuid`, `setgid`, and `setegid` modify them when
supported by the host.

### Example

```c
/* Spawn a child that prints a message and wait for it to finish. */
pid_t pid;
char *args[] = {"/bin/echo", "hello", NULL};
posix_spawn(&pid, "/bin/echo", NULL, NULL, args, environ);
waitpid(pid, NULL, 0);

/* Install a handler and send the process an interrupt. */
void on_int(int signo) { (void)signo; }
signal(SIGINT, on_int);
kill(getpid(), SIGINT);

/* Detach and run in the background. */
if (daemon(0, 0) < 0)
    perror("daemon");
```

`execvp` performs the same operation as `execve` but searches the directories in the `PATH` environment variable when the program name does not contain a slash.
`execv`, `execl`, `execlp` and `execle` are thin wrappers that build an
argument array on behalf of the caller. `execl` and `execlp` accept a
variable list of arguments terminated by `NULL`. `execle` is similar but
takes a custom environment pointer after the final `NULL` argument.

### Wait Status Helpers

Macros in `<sys/wait.h>` decode the integer status returned by `wait()`
and `waitpid()`. Use `WIFEXITED(status)` to detect normal termination
and `WEXITSTATUS(status)` to read the child's exit code. When a signal
causes termination, `WIFSIGNALED(status)` becomes true and
`WTERMSIG(status)` yields the signal number. `WIFSTOPPED(status)` tests
if the child was stopped by a signal.

The convenience `system()` call executes a shell command by forking and
invoking `/bin/sh -c command`. It returns the raw status from `waitpid`
and is intended only for simple helper tasks.
`popen` and `pclose` provide a lightweight way to run a command with a
pipe connected to the child. Use mode `"r"` to read its output or `"w"`
to send data to its stdin.
`abort()` sends `SIGABRT` to the current process and does not invoke
`atexit` handlers.
`exit()` terminates the process after running any handlers registered with `atexit()`. The handlers execute in reverse registration order. `_exit()` bypasses them.
The design favors straightforward semantics over comprehensive POSIX
conformance.

## Error Reporting

vlibc provides minimal helpers to report errors:

```c
const char *strerror(int errnum);
int strerror_r(int errnum, char *buf, size_t buflen);
void perror(const char *s);
const char *strsignal(int signum);
```


`strerror()` returns a string describing `errnum` or "Unknown error" for
codes it does not recognize. `strerror_r()` is a thread-safe variant that
writes the message into `buf`. `perror()` writes a message to `stderr`
combining the optional prefix with the text for the current `errno`.
`strsignal()` maps a signal number to a short descriptive string or
"Unknown signal" when the name is not recognized.

## Errno Access

The `errno` variable is thread‑local and stores the last failure code.
The helper `__errno_location()` returns a pointer to this value so that
macros can reference it directly.

```c
extern __thread int errno;
int *__errno_location(void);
```

## Threading

Basic thread support is delegated to the host's `pthread` library.
Only a handful of functions are provided:

```c
int pthread_create(pthread_t *thread, const void *attr,
                   void *(*start)(void *), void *arg);
int pthread_join(pthread_t thread, void **retval);
int pthread_detach(pthread_t thread);
pthread_t pthread_self(void);
int pthread_equal(pthread_t a, pthread_t b);

int pthread_mutex_init(pthread_mutex_t *mutex, void *attr);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_trylock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);

int pthread_cond_init(pthread_cond_t *cond, void *attr);
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
int pthread_cond_signal(pthread_cond_t *cond);
int pthread_cond_broadcast(pthread_cond_t *cond);

int pthread_rwlock_init(pthread_rwlock_t *rwlock, void *attr);
int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock);
int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock);
int pthread_rwlock_unlock(pthread_rwlock_t *rwlock);
int pthread_rwlock_destroy(pthread_rwlock_t *rwlock);
```

Threads share the process address space and use a simple spinlock-based
mutex for synchronization.

`pthread_self()` returns the identifier of the calling thread while
`pthread_equal()` compares two thread IDs for equality.

`pthread_create()` spawns a new thread running the `start` routine with the
given argument. The thread identifier is written to `thread` and can later be
passed to `pthread_join()` or `pthread_detach()`.

`pthread_join()` waits for a joinable thread to finish and retrieves the value
returned by the start routine. It should only be called once per thread.

`pthread_detach()` marks a thread so that its resources are reclaimed
automatically when it terminates. Detached threads cannot be joined.

Mutex routines provide minimal mutual exclusion. `pthread_mutex_init()`
initializes a mutex, `pthread_mutex_lock()` acquires it,
`pthread_mutex_trylock()` attempts to lock without blocking and returns
`EBUSY` if the mutex is already held, and `pthread_mutex_unlock()`
releases it.  Destroying a locked mutex with `pthread_mutex_destroy()` is
undefined.

Condition variables provide simple waiting semantics. A thread calls
`pthread_cond_wait()` with a locked mutex and blocks until another thread
signals the condition. `pthread_cond_signal()` wakes a single waiter while
`pthread_cond_broadcast()` wakes all waiters.

Read-write locks allow multiple threads to hold the lock in read mode or
a single writer to hold it exclusively. They are lightweight wrappers
around atomic counters and follow the same initialization and destruction
pattern as mutexes.

Thread-local storage is available through key objects:

```c
int pthread_key_create(pthread_key_t *key, void (*destructor)(void *));
int pthread_key_delete(pthread_key_t key);
int pthread_setspecific(pthread_key_t key, const void *value);
void *pthread_getspecific(pthread_key_t key);

int pthread_once(pthread_once_t *control, void (*init)(void));
```

`pthread_once` guarantees that `init` runs just once even if multiple
threads call it concurrently.

### Example

```c
#include "pthread.h"
#include <stdio.h>

static pthread_mutex_t lock;

static void *worker(void *arg) {
    int *val = arg;
    for (int i = 0; i < 1000; ++i) {
        pthread_mutex_lock(&lock);
        (*val)++;
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

int main(void) {
    pthread_t t1, t2;
    int counter = 0;

    pthread_mutex_init(&lock, NULL);
    pthread_create(&t1, NULL, worker, &counter);
    pthread_create(&t2, NULL, worker, &counter);

    pthread_join(&t1, NULL);
    pthread_join(&t2, NULL);

    printf("final: %d\n", counter);
    pthread_mutex_destroy(&lock);
    return 0;
}
```

## Dynamic Loading

The `dlfcn` module implements a minimal ELF loader. On 64-bit x86
systems it can resolve `R_X86_64_RELATIVE`, `R_X86_64_64`,
`R_X86_64_GLOB_DAT`, and `R_X86_64_JUMP_SLOT` relocations. Other
architectures are currently unsupported and attempting to load a
library will fail with an error. Use `dlopen`, `dlsym`, and `dlclose`
to load code at runtime.

## Environment Variables

The environment module exposes a global pointer `environ` storing the
process's `name=value` pairs. Programs with a custom entry point should
call `env_init(envp)` before using `getenv`, `setenv`, `putenv`, or
`unsetenv`.

```c
extern char **environ;

int main(int argc, char **argv, char **envp) {
    env_init(envp);
    setenv("FOO", "BAR", 1);
    putenv("BAR=BAZ");
    const char *v = getenv("FOO");
    unsetenv("FOO");
    return 0;
}
```

## System Information

Use `uname` from `sys/utsname.h` to query the running operating system:

```c
struct utsname u;
if (uname(&u) == 0) {
    printf("%s %s\n", u.sysname, u.release);
}
```

## Basic File I/O

Thin wrappers around the kernel's file APIs live in `io.h`. Functions
like `open`, `read`, `write`, `close`, `unlink`, `rename`, `symlink`,
`mkdir`, `rmdir`, `chdir`, and the file size controls `truncate` and
`ftruncate` simply pass their arguments to the corresponding syscalls.
`access` and `faccessat` query permissions on files without opening
them.
Vector I/O through `readv` and `writev` is available to gather or scatter
multiple buffers in a single call.

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
on platforms lacking the call.

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
int duped = dup(fd);
int pipefd[2];
pipe(pipefd);
```

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

Use `flock` to coordinate access to files between multiple processes.
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

## Secure Password Input

`getpass` reads a line from `/dev/tty` with echo disabled. The typed
password is returned in a static buffer so callers should copy it if it
needs to be preserved.

```c
char *pw = getpass("Password: ");
```

## Standard Streams

`stdin`, `stdout`, and `stderr` are lightweight streams wrapping file
descriptors 0, 1 and 2. They can be used with the provided `fread`,
`fwrite`, `fseek`, `ftell`, `fseeko`, `ftello`, `rewind`, `fgetc`, `fputc`, `ungetc`, `fgets`,
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

## Networking

The socket layer exposes thin wrappers around the kernel's networking
syscalls including `socket`, `bind`, `listen`, `accept`, `connect`,
`socketpair`, `send`, `recv`, `sendto`, `recvfrom`, `setsockopt`, and
`getsockopt`.
Address resolution is handled
via `getaddrinfo`, `freeaddrinfo`, and `getnameinfo`.

Utilities `inet_pton` and `inet_ntop` convert between IPv4 or IPv6
presentation strings and binary network format.

```c
struct addrinfo *ai;
if (getaddrinfo("localhost", "80", NULL, &ai) == 0) {
int fd = socket(AF_INET, SOCK_STREAM, 0);
    connect(fd, ai->ai_addr, ai->ai_addrlen);
    freeaddrinfo(ai);
}
```

When `getaddrinfo` fails it returns an `EAI_*` code. Use
`gai_strerror` to obtain a human readable description:

```c
struct addrinfo *ai;
int r = getaddrinfo("nosuch.host", NULL, NULL, &ai);
if (r != 0)
    fprintf(stderr, "%s\n", gai_strerror(r));
```

### Legacy Lookup API

Older programs may still rely on `gethostbyname` and `gethostbyaddr`.
vlibc implements these wrappers by consulting `/etc/hosts` and falling
back to `getaddrinfo` when no entry is found.

```c
struct hostent {
    char *h_name;
    char **h_aliases;
    int h_addrtype;
    int h_length;
    char **h_addr_list;
};

struct hostent *gethostbyname(const char *name);
struct hostent *gethostbyaddr(const void *addr, socklen_t len, int type);
```

Create a pair of connected sockets with `socketpair`:

```c
int sv[2];
if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == 0) {
    send(sv[0], "hi", 2, 0);
    char buf[3] = {0};
    recv(sv[1], buf, 2, 0);
}
```

Network interfaces can be enumerated with `getifaddrs` which fills a list of
`struct ifaddrs`. Each entry describes an interface name, flags and optional
addresses. Free the list with `freeifaddrs` when done.

```c
struct ifaddrs *ifas;
if (getifaddrs(&ifas) == 0) {
    for (struct ifaddrs *i = ifas; i; i = i->ifa_next)
        printf("%s\n", i->ifa_name);
    freeifaddrs(ifas);
}
```

## I/O Multiplexing

`select` and `poll` wait for activity on multiple file descriptors.

```c
fd_set set;
FD_ZERO(&set);
FD_SET(fd, &set);
select(fd + 1, &set, NULL, NULL, NULL);
```

On BSD systems these wrappers simply call the host C library's
implementations which may exhibit slightly different semantics than the
Linux syscall-based versions.

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
directory file descriptor.  `openat`, `unlinkat`, `mkdirat`, `symlinkat` and
`fstatat` behave like their traditional counterparts and accept `AT_FDCWD` to
use the current working directory.

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
```

On BSD systems vlibc parses the file directly. The location can be
overridden via the `VLIBC_PASSWD` environment variable for testing.

`getpwuid_r()` and `getpwnam_r()` perform the same search but store
results in caller supplied memory so they are safe for concurrent use.

`getlogin()` obtains the user name for the current UID using
`getpwuid(getuid())`.  The resulting string is cached in thread-local
storage so repeated calls are inexpensive.

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
```

As with the password file, BSD platforms parse the group database directly.
The path can be overridden via the `VLIBC_GROUP` environment variable when
running tests. The `*_r` variants fill caller provided buffers and are
thread-safe.

## Time Formatting

`strftime` understands `%Y`, `%m`, `%d`, `%H`, `%M`, and `%S` along with `%a`, `%b`, `%Z`, `%z`, and weekday numbers (`%w`/`%u`).
`strptime` still parses the numeric fields (`%Y`, `%m`, `%d`, `%H`, `%M`, `%S`) back into a `struct tm` and returns a pointer to the first unparsed character or `NULL` on failure.

Example:

```c
struct tm tm = {
    .tm_year = 123, // 2023
    .tm_mon  = 4,   // May
    .tm_mday = 6,
    .tm_wday = 6,
    .tm_hour = 7,
    .tm_min  = 8,
    .tm_sec  = 9
};
char buf[64];
strftime(buf, sizeof(buf), "%a %b %d %Y %H:%M:%S %Z %z", &tm);
// "Sat May 06 2023 07:08:09 UTC +0000"
```
`timegm` converts a `struct tm` in UTC back to `time_t` using the same logic as `mktime` but without timezone adjustments.

## Locale Support

`setlocale` reads `LC_ALL` and `LANG` and sets the active locale
accordingly. For non-`"C"` locales the function defers to the host
`setlocale(3)` implementation on BSD systems. `localeconv` returns
formatting information. `gmtime`, `localtime`, `mktime`, and `ctime`
convert between `time_t` and `struct tm` or human readable strings.

## Time Retrieval

Use `time`, `gettimeofday`, or `clock_gettime` to obtain the current time of day.

```c
time_t now = time(NULL);
struct timespec ts;
clock_gettime(CLOCK_REALTIME, &ts);
```

The resolution of a clock can be determined with `clock_getres`:

```c
struct timespec res;
clock_getres(CLOCK_MONOTONIC, &res);
```

Thread-safe variants `gmtime_r` and `localtime_r` fill a user-provided
`struct tm` using the same conversion logic.  `tzset` updates the active
timezone by reading the `TZ` environment variable on BSD systems.

## Sleep Functions

Delay helpers are available in `time.h`:

```c
unsigned sleep(unsigned seconds);
int usleep(useconds_t usec);
int nanosleep(const struct timespec *req, struct timespec *rem);
unsigned int alarm(unsigned int seconds);
```

## Interval Timers

`setitimer` schedules periodic `SIGALRM` delivery or CPU timers. `getitimer`
returns the remaining time and interval.

```c
struct itimerval it = { {1, 0}, {1, 0} };
setitimer(ITIMER_REAL, &it, NULL);
```

## Resource Limits

Processes may query and update operating system limits using
`getrlimit` and `setrlimit` from `sys/resource.h`.

```c
struct rlimit lim;
if (getrlimit(RLIMIT_NOFILE, &lim) == 0) {
    printf("soft: %lu hard: %lu\n",
           (unsigned long)lim.rlim_cur,
           (unsigned long)lim.rlim_max);
}
```

## Logging

`syslog.h` provides simple helpers to send log messages to `/dev/log` on
BSD-style systems. Call `openlog` once with an identifier then use `syslog`
with a priority and `printf`-style format:

```c
openlog("myapp", LOG_PID, LOG_USER);
syslog(LOG_INFO, "started with %d workers", workers);
closelog();
```

The underlying function `vsyslog` accepts a `va_list` so you can wrap the
logger with custom helpers:

```c
static void debug(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsyslog(LOG_DEBUG, fmt, ap);
    va_end(ap);
}
```

Messages are written using a Unix datagram socket so applications can integrate
with the host's syslog daemon.

## Raw System Calls

The `syscall` function from `syscall.h` invokes the host's `syscall()`
wrapper when available. Platform specific implementations may override
this if the operating system lacks a compatible interface.

## Non-local Jumps

When possible vlibc defers to the host C library's `setjmp` and `longjmp`.
For targets lacking a native implementation, custom versions live under
`src/arch/<arch>/setjmp.c`.

```c
int setjmp(jmp_buf env);
void longjmp(jmp_buf env, int val);
```

Jumping across signal handlers may leave blocked signals in an undefined
state.

## Limitations

 - The I/O routines perform simple optional buffering and provide only
   basic error reporting.
 - Process creation currently relies on Linux-specific syscalls.
 - BSD support is experimental and some subsystems may not compile yet.
 - The `system()` helper spawns `/bin/sh -c` and lacks detailed status
   codes.
 - `perror` and `strerror` cover only common errors.
 - Thread support is limited to basic mutexes and join/detach.
 - Locale handling falls back to the host implementation for values other
   than `"C"` or `"POSIX"`.
 - `setjmp`/`longjmp` rely on the host C library when available.
   Only an x86_64 fallback implementation is provided.
 - Regular expressions cover only a subset of POSIX syntax. Capture
   groups and numeric backreferences are supported but more advanced
   features remain unimplemented.

## Conclusion

vlibc is intentionally small and focused. This documentation will evolve as the project grows, but the guiding principles of minimalism and clarity will remain the same. Contributions are welcome as long as they align with these goals.

