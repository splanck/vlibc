# vlibc Documentation

This document outlines the architecture, planned modules, and API design for **vlibc**. The goal is to provide a clear overview for contributors and users who wish to understand how the library is organized.

## Table of Contents

1. [Initialization](#initialization)
2. [Memory Management](#memory-management)
3. [Input/Output](#inputoutput)
4. [String Handling](#string-handling)
5. [Process Control](#process-control)

## Architecture

vlibc aims to implement a minimal subset of the standard C library. The project is structured around small, easily auditable modules. Each module exposes a minimal API and relies on direct system calls whenever possible. The code is written in a straightforward C style without complex build systems.

The overall repository layout is described in the README. In short, source code
lives in `src/`, public headers in `include/`, and tests will be placed in
`tests/`.

### Planned Modules

- **startup**: Program entry, stack setup, and initialization routines.
- **memory**: Basic heap management using `brk`/`sbrk` or a small memory allocator.
- **io**: Simple wrappers around read/write system calls for file descriptors.
- **string**: Common string operations such as length checks and copying.
- **process**: Functions for spawning and waiting on child processes.

## API Design

vlibc strives for a consistent API surface inspired by the C standard library. Functions are prefixed with `v` (e.g., `vmalloc`) to differentiate them from their libc counterparts. Where appropriate, functions mirror POSIX signatures so users can easily migrate existing code.

### Naming Conventions

- `v` prefix for all exported functions.
- Lowercase with underscores for function names and variables.
- Keep parameter lists short and intuitive.

## Initialization

Initialization code lives in the **startup** module. It performs the following tasks:

1. Set up the runtime stack and registers.
2. Parse arguments and environment variables.
3. Call `v_main()` as the program entry point.
4. Provide a minimal exit routine to flush data and terminate the process.

Programs using vlibc are expected to define a `v_main()` function that receives the traditional `argc`, `argv`, and `envp` parameters. This keeps the startup routine simple and avoids pulling in complex features like C++ static constructors.

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

## Input/Output

vlibc includes simple wrappers for the fundamental POSIX file APIs:

```c
int open(const char *path, int flags, mode_t mode);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
int close(int fd);
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
APIs.

## String Handling

The **string** module provides fundamental operations needed by most C programs:

- `vstrlen`, `vstrcpy`, and `vstrncmp` equivalents.
- Conventional memory routines (`memcpy`, `memmove`, `memset`, `memcmp`) map to
  the internal `v` implementations.
- Minimal locale or encoding support; all strings are treated as byte sequences.
- Utility functions for tokenizing and simple formatting.
- Simple number conversion helpers `atoi` and `strtol`.

The goal is to offer just enough functionality for common tasks without the complexity of full locale-aware libraries.

## Process Control

Process-related functionality resides in the **process** module. It provides
minimal wrappers for creating and managing processes as well as installing
signal handlers:

```c
pid_t fork(void);
int execve(const char *pathname, char *const argv[], char *const envp[]);
pid_t waitpid(pid_t pid, int *status, int options);
int kill(pid_t pid, int sig);
sighandler_t signal(int signum, sighandler_t handler);
int system(const char *command);
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

The convenience `system()` call executes a shell command by forking and
invoking `/bin/sh -c command`. It returns the raw status from `waitpid`
and is intended only for simple helper tasks.


The design favors straightforward semantics over comprehensive POSIX
conformance.

## Threading

Basic thread support is implemented on top of the Linux `clone` syscall.
Only a handful of functions are provided:

```c
int pthread_create(pthread_t *thread, const void *attr,
                   void *(*start)(void *), void *arg);
int pthread_join(pthread_t *thread, void **retval);

int pthread_mutex_init(pthread_mutex_t *mutex, void *attr);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
```

Threads share the process address space and use a simple spinlock-based
mutex for synchronization.

## Conclusion

vlibc is intentionally small and focused. This documentation will evolve as the project grows, but the guiding principles of minimalism and clarity will remain the same. Contributions are welcome as long as they align with these goals.

