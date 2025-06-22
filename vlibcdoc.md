# vlibc Documentation

This document outlines the architecture, planned modules, and API design for **vlibc**. The goal is to provide a clear overview for contributors and users who wish to understand how the library is organized.

## Table of Contents
1. [Memory Management](#memory-management)
2. [Input/Output](#inputoutput)
3. [String Handling](#string-handling)
4. [Process Control](#process-control)
5. [Error Reporting](#error-reporting)
6. [Threading](#threading)
7. [Dynamic Loading](#dynamic-loading)


## Memory Management

The **memory** module provides a very small heap allocator implemented in
`memory.c`. It relies on the `sbrk` system call to extend the heap and keeps
the implementation deliberately simple. Each allocation stores a small header
so the most recent block can be released on `free()`. Memory for older blocks
is still not recycled, keeping the code easy to audit at the cost of efficiency.

### API

```c
void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
```

### Behavior and Caveats

- `malloc` allocates memory linearly using `sbrk` and returns `NULL` on
  failure.
- `calloc` calls `malloc` and zeroes the allocated block.
- `realloc` always allocates a new block and copies up to `size` bytes from the
  old pointer if one was provided.
- `free` releases memory only if called on the most recently allocated block.

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

## Input/Output

vlibc includes simple wrappers for the fundamental POSIX file APIs:

```c
int open(const char *path, int flags, mode_t mode);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
int close(int fd);
int unlink(const char *pathname);
int rename(const char *oldpath, const char *newpath);
int symlink(const char *target, const char *linkpath);
int mkdir(const char *pathname, mode_t mode);
int rmdir(const char *pathname);
int chdir(const char *path);
```

These functions forward their arguments directly to the kernel using the syscall interface. No buffering or stream abstraction is performed.

### Example

```c
int fd = open("log.txt", O_WRONLY | O_CREAT, 0644);
if (fd >= 0) {
    write(fd, "hello\n", 6);
    close(fd);
}
```

The stdio module also exposes `stdin`, `stdout`, and `stderr` as global
pointers. These streams wrap file descriptors 0, 1 and 2 and are
initialized in `vlibc_init()` so they can be used with the basic FILE
APIs. Available helpers include `fopen`, `fread`, `fwrite`, `fseek`,
`ftell`, `rewind`, `fclose`, `fgetc`, `fputc`, `fgets`, `fputs`,
`sprintf`, `snprintf`, `vsprintf`, `vsnprintf`, `fprintf`, `vfprintf`,
`printf`, and `vprintf`.
`fflush`, and simple formatted output via `fprintf` and `printf`.
Because the library does not buffer stream data, `fflush` simply
performs an `fsync` on the given stream when it is non-`NULL` and
otherwise returns success.

## String Handling

The **string** module provides fundamental operations needed by most C programs:

- `vstrlen`, `vstrcpy`, `vstrncmp`, `strnlen`, `strcat` and `strncat` equivalents.
- Search helpers `strstr`, `strrchr`, and `memchr` for locating substrings or bytes.
- Conventional memory routines (`memcpy`, `memmove`, `memset`, `memcmp`) map to
  the internal `v` implementations.
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
`abort()` sends `SIGABRT` to the current process and does not invoke
`atexit` handlers.
`exit()` terminates the process after running any handlers registered with `atexit()`. The handlers execute in reverse registration order. `_exit()` bypasses them.
The design favors straightforward semantics over comprehensive POSIX
conformance.

## Networking

vlibc currently provides minimal socket wrappers for basic network
communication:

```c
int socket(int domain, int type, int protocol);
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
ssize_t send(int sockfd, const void *buf, size_t len, int flags);
ssize_t recv(int sockfd, void *buf, size_t len, int flags);
ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest, socklen_t addrlen);
ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                 struct sockaddr *src, socklen_t *addrlen);
int select(int nfds, fd_set *readfds, fd_set *writefds,
           fd_set *exceptfds, struct timeval *timeout);
int poll(struct pollfd *fds, nfds_t nfds, int timeout);
int getaddrinfo(const char *node, const char *service,
                const struct addrinfo *hints, struct addrinfo **res);
void freeaddrinfo(struct addrinfo *res);
int getnameinfo(const struct sockaddr *sa, socklen_t salen,
                char *host, socklen_t hostlen,
                char *serv, socklen_t servlen, int flags);
```

These wrappers directly invoke the underlying `socket`, `bind`,
`connect`, `sendto`, and `recvfrom` syscalls without additional
buffering or complex address handling.  Address lookups can be
performed with `getaddrinfo` and `getnameinfo`:

```c
struct addrinfo *ai;
if (getaddrinfo("127.0.0.1", "8080", NULL, &ai) == 0) {
    connect(fd, ai->ai_addr, ai->ai_addrlen);
    freeaddrinfo(ai);
}
```

## Error Reporting

vlibc provides minimal helpers to report errors:

```c
const char *strerror(int errnum);
void perror(const char *s);
```

`strerror()` returns a string describing `errnum` or "Unknown error" for
codes it does not recognize. `perror()` writes a message to `stderr`
combining the optional prefix with the text for the current `errno`.

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

## Conclusion

vlibc is intentionally small and focused. This documentation will evolve as the project grows, but the guiding principles of minimalism and clarity will remain the same. Contributions are welcome as long as they align with these goals.

