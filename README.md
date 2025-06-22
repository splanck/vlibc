# vlibc

vlibc is a minimal libc replacement targeted at UNIX-like systems. It
provides a small subset of the standard C library with an emphasis on
simplicity and predictability. The project is distributed under the
terms of the GNU General Public License v3.0.

## Goals
- Keep the code base small and auditable.
- Supply only the essential C runtime facilities.
- Offer consistent behavior for statically linked or embedded programs.

## Target Platforms
- Linux (x86_64, aarch64, armv7).
- Other POSIX systems with minimal porting effort.
- Lightweight containers or small research kernels that implement POSIX
  style system calls.

## Guiding Principles
- Minimalism: implement just enough functionality for common use.
- Readability over clever micro-optimizations.
- Prefer direct system calls and avoid heavy runtime features.
- Encourage static linking to reduce external dependencies.

## Motivation and Use Cases
vlibc was created for developers who need a tiny C runtime for
experiments, educational operating systems, or compact container
images. It serves as a foundation for projects that require libc
functionality but do not want the complexity of larger implementations.

## Differences from Existing Solutions
While glibc and musl provide comprehensive POSIX support, vlibc focuses
on a much smaller API surface. It is **not** a drop-in replacement for
the full standard library. Instead, vlibc offers a minimal set of
wrappers and utilities that can be easily inspected, modified, or
extended. This makes it suitable for specialized applications where the
complete feature set of other libcs is unnecessary.

## Project Structure

The repository uses a straightforward layout:

- `src/` contains the library's source files.
- `include/` holds public header files.
- `tests/` contains unit tests.

Common memory routines (`memcpy`, `memmove`, `memset`, `memcmp`) are available
as wrappers around the internal `v*` implementations so existing code can use
the familiar names.  The tiny allocator in `memory.c` now tracks all freed
blocks and coalesces adjacent regions so subsequent calls to `malloc` and
`realloc` can reuse memory more effectively.
Basic string helpers like `strcmp`, `strchr`, `strncpy`, `strdup`, and
`strnlen` are also provided.

## Provided Headers

vlibc installs a small set of public headers for application use:

```
ctype.h      - character classification helpers
dirent.h     - directory iteration
env.h        - environment variable access
errno.h      - standard error codes
io.h         - unbuffered I/O primitives
memory.h     - heap allocation
math.h       - basic math routines
process.h    - process creation and control
pthread.h    - minimal threading support
poll.h       - I/O multiplexing helpers
dlfcn.h      - runtime loading of shared libraries
stdio.h      - simple stream I/O
stdlib.h     - basic utilities
string.h     - string manipulation
wchar.h     - wide character helpers
getopt.h     - option parsing
sys/mman.h   - memory mapping helpers
sys/socket.h - networking wrappers
sys/file.h   - file permission helpers
netdb.h      - address resolution helpers
sys/stat.h   - file status functions
syscall.h    - raw syscall interface
time.h       - time related helpers
setjmp.h     - non-local jump helpers
vlibc.h      - library initialization
```

## Building the Library

The project uses a simple `make`-based build system. To compile the
static library, run:

```sh
make
```

This produces `libvlibc.a` in the repository root. You can optionally
install the headers and library with:

```sh
make install PREFIX=/usr/local
```

`PREFIX` controls the installation path and defaults to `/usr/local`.

To link against vlibc in your application, add the following flags when
compiling:

```sh
cc your_app.c -I/path/to/vlibc/include -L/path/to/vlibc -lvlibc
```

## Running Tests

Unit tests live in `tests/` and use a tiny test harness. The suite aims to
exercise the library's core functions. Build and execute the tests with:

```sh
make test
```

This command builds `tests/run_tests` and runs it automatically.

## String Conversion

vlibc provides simple helpers to convert strings into integers. Use
`atoi()` for basic decimal parsing or `strtol()` when you need other
bases or the end pointer. Floating-point numbers can be parsed with
`strtod()` or the convenience wrapper `atof()`.

```c
int v = atoi("123");            /* v == 123 */
char *end;
long x = strtol("ff", &end, 16); /* x == 255 and *end == '\0' */
double f = strtod("3.14", NULL); /* f == 3.14 */
```

Use `strcat()` or `strncat()` to append one string to another.
Search helpers `strstr()`, `strrchr()`, and `memchr()` locate substrings or
individual bytes within a buffer.

## String Tokenization

`strtok()` splits a string into tokens separated by any characters in the
delimiter set. The function keeps its state in static storage, so it is not
thread-safe or reentrant. Call `strtok(NULL, delim)` to continue scanning the
same string. When concurrency is needed, prefer `strtok_r` which lets the
caller manage the context.

```c
char buf[] = "a b c";
char *tok = strtok(buf, " ");
while (tok) {
    printf("%s\n", tok);
    tok = strtok(NULL, " ");
}
```

## Sorting Helpers

`qsort()` sorts an array in-place using a user-provided comparison
function, while `bsearch()` performs binary search on a sorted array.

```c
int values[] = {4, 2, 7};
qsort(values, 3, sizeof(int), cmp_int);
int key = 7;
int *found = bsearch(&key, values, 3, sizeof(int), cmp_int);
```

## Option Parsing

`getopt()` iterates over command-line options. A character followed by `:` in the
option string expects an argument. Parsing stops at the first non-option or `--`.

```c
int flag = 0;
char *arg = NULL;
int c;
while ((c = getopt(argc, argv, "fa:")) != -1) {
    switch (c) {
    case 'f':
        flag = 1;
        break;
    case 'a':
        arg = optarg;
        break;
    default:
        /* unknown option */
        break;
    }
}
```

`getopt_long()` behaves similarly but accepts an array of `struct option` to
describe GNU-style long options:

```c
static const struct option longopts[] = {
    {"flag",  no_argument,       NULL, 'f'},
    {"alpha", required_argument, NULL, 'a'},
    {0, 0, 0, 0}
};
while ((c = getopt_long(argc, argv, "fa:", longopts, NULL)) != -1) {
    ...
}
```

## Standard Streams

vlibc's stdio layer exposes global pointers `stdin`, `stdout`, and
`stderr`. These lightweight streams wrap file descriptors 0, 1 and 2 and
are initialized when `vlibc_init()` is called. They can be used with the
provided `fread`, `fwrite`, `fseek`, `ftell`, `rewind`, `fgetc`,

`fputc`, `fgets`, `fputs`, `sprintf`, `snprintf`, `vsprintf`, `vsnprintf`,
`fprintf`, `vfprintf`, `printf`, and `vprintf` functions.

`fputc`, `fgets`, `fputs`, `fflush`, `fprintf`, and `printf` functions.
Although I/O is unbuffered, `fflush(stream)` succeeds and invokes
`fsync` on the stream's file descriptor when one is supplied.


## Networking

The socket layer exposes thin wrappers around the kernel's networking
syscalls. Available functions include `socket`, `bind`, `listen`,
`accept`, `connect`, `send`, `recv`, `sendto`, `recvfrom`, as well as
the I/O multiplexing helpers `select` and `poll`.  Basic address
resolution is provided through `getaddrinfo`, `freeaddrinfo`, and
`getnameinfo`.

These calls accept the same arguments as their POSIX counterparts and
translate directly to the underlying `socket`, `bind`, `connect`, and
`sendto`/`recvfrom` syscalls.  Example:

```c
struct addrinfo *ai;
if (getaddrinfo("localhost", "80", NULL, &ai) == 0) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    connect(fd, ai->ai_addr, ai->ai_addrlen);
    freeaddrinfo(ai);
}
```

## Dynamic Loading

vlibc ships with a tiny ELF loader that understands position-independent
shared objects on Linux. Applications can call `dlopen`, `dlsym`, and
`dlclose` from `dlfcn.h` to load modules at runtime.

```c
void *h = dlopen("plugin.so", RTLD_NOW);
int (*fn)(void) = dlsym(h, "plugin_value");
fn();
dlclose(h);
```

## Error Reporting

Two helpers make it easier to display error messages:

```c
const char *strerror(int errnum);
void perror(const char *s);
```

`strerror()` returns a string for a known error code, while `perror()`
prints the current `errno` value with an optional prefix.

## File Permissions

The file module exposes simple wrappers to adjust permissions and ownership.

```c
umask(022);
chmod("data.txt", 0644);
chown("data.txt", 1000, 1000);
```

## Process Control

The process module forwards common process-management calls directly to the kernel. Wrappers are available for `fork`, `execve`, `execvp`, `waitpid`, `kill`, `getpid`, `getppid`, `signal`, and `abort`. A simple `system()` convenience function is also included.

`execvp` searches the directories listed in the `PATH` environment variable and
then invokes `execve` on the first matching program.

A call to `abort()` sends `SIGABRT` to the process and terminates it immediately.
A lightweight `popen`/`pclose` pair runs a shell command with a pipe
connected to the child. Use mode `"r"` to read the command's output or
`"w"` to send data to its stdin:

```c
FILE *f = popen("echo hi", "r");
char buf[16] = {0};
fread(buf, 1, sizeof(buf) - 1, f);
pclose(f);
```

## Time Formatting

The library includes a minimal `strftime` implementation for producing
human-readable timestamps. Supported conversion sequences are `%Y`, `%m`, `%d`,
`%H`, `%M`, and `%S`. Locale handling is extremely limited and only the default
`"C"` and `"POSIX"` locales can be selected.

## Locale Support

`setlocale` can switch between the built-in `"C"` and `"POSIX"` locales.
`localeconv` returns formatting information for these locales only.

Basic conversion helpers `gmtime`, `localtime`, `mktime`, and `ctime` are
provided for transforming between `time_t` values and `struct tm` or
human-readable strings. `localtime` does not apply any timezone logic and
behaves identically to `gmtime`.


## Limitations

 - The I/O routines (`open`, `read`, `write`, `close`, `unlink`, `rename`, `symlink`, `mkdir`, `rmdir`, `chdir`) are thin wrappers around
  the corresponding system calls. They perform no buffering and provide only
  basic error reporting.
- Process creation and signal functions rely on Linux `fork`, `execve`,
  `wait4`/`waitpid`, and `rt_sigaction` syscalls. Porting to other UNIX-like
  kernels may require adapting these calls.
- The `system()` helper simply spawns `/bin/sh -c` in a child process.
  It does not handle complex quoting or return detailed status codes.
- `perror` and `strerror` cover only common error codes.
- Basic thread support is implemented using the `clone` syscall. Only
  `pthread_create`, `pthread_join`, and simple mutexes
  (`pthread_mutex_init`, `pthread_mutex_destroy`,
  `pthread_mutex_lock`, `pthread_mutex_unlock`) are provided.
- Locale data is minimal: only the `"C"` and `"POSIX"` locales are supported.
- `setjmp`/`longjmp` only work on x86_64 and do not preserve signal masks.
