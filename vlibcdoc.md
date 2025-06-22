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
11. [Math Functions](#math-functions)
12. [Process Control](#process-control)
13. [Error Reporting](#error-reporting)
14. [Errno Access](#errno-access)
15. [Threading](#threading)
16. [Dynamic Loading](#dynamic-loading)
17. [Environment Variables](#environment-variables)
18. [Basic File I/O](#basic-file-io)
19. [File Descriptor Helpers](#file-descriptor-helpers)
20. [Standard Streams](#standard-streams)
21. [Networking](#networking)
22. [I/O Multiplexing](#io-multiplexing)
23. [File Permissions](#file-permissions)
24. [File Status](#file-status)
25. [Directory Iteration](#directory-iteration)
26. [Time Formatting](#time-formatting)
27. [Locale Support](#locale-support)
28. [Time Retrieval](#time-retrieval)
29. [Sleep Functions](#sleep-functions)
30. [Raw System Calls](#raw-system-calls)
31. [Non-local Jumps](#non-local-jumps)
32. [Limitations](#limitations)
33. [Conclusion](#conclusion)

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
- Linux (x86_64, aarch64, armv7).
- Other POSIX systems with minimal porting effort.
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
- **math**: Elementary math routines like `sin` and `sqrt`.

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
netdb.h      - address resolution helpers
poll.h       - I/O multiplexing helpers
process.h    - process creation and control
pthread.h    - minimal threading support
setjmp.h     - non-local jump helpers
stdio.h      - simple stream I/O
stdlib.h     - basic utilities
string.h     - string manipulation
sys/file.h   - file permission helpers
sys/mman.h   - memory mapping helpers
sys/select.h - fd_set macros and select wrapper
sys/socket.h - networking wrappers
sys/stat.h   - file status functions
syscall.h    - raw syscall interface
time.h       - time related helpers
vlibc.h      - library initialization
wchar.h      - wide character helpers
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
```

`mmap` creates new mappings, `munmap` releases them and `mprotect` changes
their access protections.

## String Handling

The **string** module provides fundamental operations needed by most C programs:

- `vstrlen`, `vstrcpy`, `vstrncmp`, `strnlen`, `strcat` and `strncat` equivalents.
- Search helpers `strstr`, `strrchr`, and `memchr` for locating substrings or bytes.
- Conventional memory routines (`memcpy`, `memmove`, `memset`, `memcmp`) map to
  the internal `v` implementations.
- The low-level memory helpers `vmemcpy`, `vmemmove`, `vmemset`, and `vmemcmp` operate on raw byte buffers. `vmemcpy` copies bytes from a source to a destination, `vmemmove` handles overlaps safely, `vmemset` fills a region with a byte value, and `vmemcmp` compares two buffers. The standard `memcpy`, `memmove`, `memset`, and `memcmp` functions simply call these implementations.
- Basic locale handling is limited to the built-in `"C"` and `"POSIX"` locales.
   `setlocale` switches between them and `localeconv` exposes formatting data.
   All strings are treated as byte sequences.
- Utility functions for tokenizing and simple formatting.
- `strtok` and `strtok_r` split a string into tokens based on a set of
  delimiter characters. `strtok` stores its parsing state in static
  memory and is not thread-safe. `strtok_r` lets the caller maintain the
  context and is safe for concurrent use.
- Simple number conversion helpers `atoi`, `strtol`, `strtod`, and `atof`.

Basic time formatting is available via `strftime`. Only a small subset of
 conversions is implemented (`%Y`, `%m`, `%d`, `%H`, `%M`, `%S`) and the
 output uses the current locale (only `"C"`/`"POSIX"` are available).

The library also includes simple conversion routines `gmtime`, `localtime`,
`mktime`, and `ctime`. They convert between `time_t` and `struct tm` or
produce a readable string. `localtime` ignores the system timezone so the
result is identical to `gmtime`.

The goal is to offer just enough functionality for common tasks without the complexity of full locale-aware libraries.

### Wide Character Conversion

`mbtowc` converts a multibyte sequence to a single `wchar_t` and `wctomb`
performs the opposite conversion. `wcslen` returns the length of a wide
string excluding the terminator.

## Character Classification

Character checks live in [include/ctype.h](include/ctype.h).  The macros
operate on a small table of 128 entries, so only the standard ASCII
characters are recognized.

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

Command-line arguments can be processed with `getopt` or `getopt_long`. The
former handles short options while the latter accepts an array of `struct
option` describing long names. `getopt_long` returns the option's value field
or sets a flag when supplied in the table.

## Random Numbers

vlibc provides a minimal pseudo-random number generator implemented as a
linear congruential generator.

```c
int rand(void);
void srand(unsigned seed);
```

Calling `srand()` initializes the internal state. Reusing the same seed
produces the identical sequence of numbers, each in the range `0` to
`32767`.

## Sorting Helpers

`qsort` sorts an array in place using a user-supplied comparison
function while `bsearch` performs binary search on a sorted array.

```c
int values[] = {4, 2, 7};
qsort(values, 3, sizeof(int), cmp_int);
int key = 7;
int *found = bsearch(&key, values, 3, sizeof(int), cmp_int);
```

## Math Functions

`sin`, `cos`, `tan`, `sqrt`, and `pow` are provided in `math.h`.

## Process Control

Process-related functionality resides in the **process** module. It provides
minimal wrappers for creating and managing processes, querying process IDs,
and installing signal handlers:

```c
pid_t fork(void);
int execve(const char *pathname, char *const argv[], char *const envp[]);
int execvp(const char *file, char *const argv[]);
pid_t waitpid(pid_t pid, int *status, int options);
int kill(pid_t pid, int sig);
pid_t getpid(void);
pid_t getppid(void);
sighandler_t signal(int signum, sighandler_t handler);
int system(const char *command);
int atexit(void (*fn)(void));
void abort(void);
void exit(int status);
```

### Example

```c
/* Spawn a child that prints a message and wait for it to finish. */
pid_t pid = fork();
if (pid == 0) {
    char *args[] = {"/bin/echo", "hello", NULL};
    execve("/bin/echo", args, NULL);
}
waitpid(pid, NULL, 0);

/* Install a handler and send the process an interrupt. */
void on_int(int signo) { (void)signo; }
signal(SIGINT, on_int);
kill(getpid(), SIGINT);
```

`execvp` performs the same operation as `execve` but searches the directories in the `PATH` environment variable when the program name does not contain a slash.

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
void perror(const char *s);
```


`strerror()` returns a string describing `errnum` or "Unknown error" for
codes it does not recognize. `perror()` writes a message to `stderr`
combining the optional prefix with the text for the current `errno`.

## Errno Access

The global `errno` variable stores the last failure code. The helper
`__errno_location()` returns a pointer to this value so that macros can
reference it directly.

```c
extern int errno;
int *__errno_location(void);
```

## Threading

Basic thread support is implemented on top of the Linux `clone` syscall.
Only a handful of functions are provided:

```c
int pthread_create(pthread_t *thread, const void *attr,
                   void *(*start)(void *), void *arg);
int pthread_join(pthread_t *thread, void **retval);
int pthread_detach(pthread_t *thread);

int pthread_mutex_init(pthread_mutex_t *mutex, void *attr);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
```

Threads share the process address space and use a simple spinlock-based
mutex for synchronization.

`pthread_create()` spawns a new thread running the `start` routine with the
given argument.  The thread ID is written to `thread` and can later be passed
to `pthread_join()` or `pthread_detach()`.

`pthread_join()` waits for a joinable thread to finish and retrieves the value
returned by the start routine. It should only be called once per thread.

`pthread_detach()` marks a thread so that its resources are reclaimed
automatically when it terminates. Detached threads cannot be joined.

Mutex routines provide minimal mutual exclusion. `pthread_mutex_init()`
initializes a mutex, `pthread_mutex_lock()` acquires it, and
`pthread_mutex_unlock()` releases it.  Destroying a locked mutex with
`pthread_mutex_destroy()` is undefined.

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

The `dlfcn` module implements a minimal ELF loader. Only the
`R_X86_64_RELATIVE` relocation type is supported, which is enough for
simple position independent libraries. Use `dlopen`, `dlsym`, and
`dlclose` to load code at runtime.

## Environment Variables

The environment module exposes a global pointer `environ` storing the
process's `name=value` pairs. Programs with a custom entry point should
call `env_init(envp)` before using `getenv`, `setenv`, or `unsetenv`.

```c
extern char **environ;

int main(int argc, char **argv, char **envp) {
    env_init(envp);
    setenv("FOO", "BAR", 1);
    const char *v = getenv("FOO");
    unsetenv("FOO");
    return 0;
}
```

## Basic File I/O

Thin wrappers around the kernel's file APIs live in `io.h`. Functions
like `open`, `read`, `write`, `close`, `unlink`, `rename`, `symlink`,
`mkdir`, `rmdir`, and `chdir` simply pass their arguments to the
corresponding syscalls.

```c
int fd = open("log.txt", O_WRONLY | O_CREAT, 0644);
if (fd >= 0) {
    write(fd, "hello\n", 6);
    close(fd);
}
```

## File Descriptor Helpers

Low-level descriptor routines perform simple tasks such as repositioning a file
or duplicating handles.

```c
off_t pos = lseek(fd, 0, SEEK_SET);
int duped = dup(fd);
int pipefd[2];
pipe(pipefd);
```

## Standard Streams

`stdin`, `stdout`, and `stderr` are lightweight streams wrapping file
descriptors 0, 1 and 2. They can be used with the provided `fread`,
`fwrite`, `fseek`, `ftell`, `rewind`, `fgetc`, `fputc`, `fgets`,
`fputs`, `sprintf`, `snprintf`, `vsprintf`, `vsnprintf`, `fprintf`,
`vfprintf`, `printf`, and `vprintf` helpers. `fflush(stream)` succeeds
and invokes `fsync` on the descriptor when one is present.

## Networking

The socket layer exposes thin wrappers around the kernel's networking
syscalls including `socket`, `bind`, `listen`, `accept`, `connect`,
`send`, `recv`, `sendto`, and `recvfrom`. Address resolution is handled
via `getaddrinfo`, `freeaddrinfo`, and `getnameinfo`.

```c
struct addrinfo *ai;
if (getaddrinfo("localhost", "80", NULL, &ai) == 0) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    connect(fd, ai->ai_addr, ai->ai_addrlen);
    freeaddrinfo(ai);
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

## File Permissions

Simple helpers adjust permissions and ownership:

```c
umask(022);
chmod("data.txt", 0644);
chown("data.txt", 1000, 1000);
```

## File Status

`stat`, `fstat`, and `lstat` from `sys/stat.h` query file metadata.

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

## Time Formatting

A minimal `strftime` supports `%Y`, `%m`, `%d`, `%H`, `%M`, and `%S`.

## Locale Support

`setlocale` switches between the built-in `"C"` and `"POSIX"` locales and
`localeconv` returns formatting information. `gmtime`, `localtime`,
`mktime`, and `ctime` convert between `time_t` and `struct tm` or human
readable strings.

## Time Retrieval

Use `time` or `gettimeofday` to obtain the current time of day.

```c
time_t now = time(NULL);
struct timeval tv;
gettimeofday(&tv, NULL);
```

## Sleep Functions

Delay helpers are available in `time.h`:

```c
unsigned sleep(unsigned seconds);
int usleep(useconds_t usec);
int nanosleep(const struct timespec *req, struct timespec *rem);
```

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

 - The I/O routines perform no buffering and provide only basic error
   reporting.
 - Process creation relies on Linux specific syscalls.
 - The `system()` helper spawns `/bin/sh -c` and lacks detailed status
   codes.
 - `perror` and `strerror` cover only common errors.
 - Thread support is limited to basic mutexes and join/detach.
 - Only the `"C"` and `"POSIX"` locales are built in.
 - `setjmp`/`longjmp` rely on the host C library when available.
   Only an x86_64 fallback implementation is provided.

## Conclusion

vlibc is intentionally small and focused. This documentation will evolve as the project grows, but the guiding principles of minimalism and clarity will remain the same. Contributions are welcome as long as they align with these goals.

