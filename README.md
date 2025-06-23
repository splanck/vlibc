# vlibc

vlibc aims to provide a tiny C standard library for UNIX-like systems.
Its design keeps things simple and favors static linking so binaries
remain self-contained.

Designed as a minimal libc implementation for UNIX-like systems, vlibc
packs only the most essential runtime components needed by portable C
programs. Key features include:

- File I/O wrappers
- Process creation and control
- Threading primitives
- Networking sockets
- Dynamic loading
- Environment variable handling
- Host name queries and changes

**Note**: vlibc provides only a small subset of the standard C library. Some
functions depend on system calls that are currently implemented for Linux. BSD
variants are now partially supported and the memory mapping routines fall back
to `MAP_ANON` when `MAP_ANONYMOUS` is unavailable. A few features may still
rely on platform-specific interfaces.

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

For detailed documentation, see [vlibcdoc.md](vlibcdoc.md).

## Time Retrieval

Use `clock_gettime` for precise timestamps.

```c
struct timespec ts;
clock_gettime(CLOCK_MONOTONIC, &ts);
```

`CLOCK_REALTIME` returns the wall-clock time.

## IPv6 Support

Networking helpers such as `inet_pton`, `inet_ntop`, `getaddrinfo` and
`getnameinfo` understand both IPv4 and IPv6 addresses. Use the standard
`AF_INET6` family to work with IPv6 sockets and address resolution.

## Platform Support

The library currently targets Linux but aims to run on other POSIX systems as
well. BSD compatibility has been tested on FreeBSD, though some modules still
rely on Linux-only system calls. Non-Linux builds may therefore require
additional work.

## Running Tests

Example tests live in the `tests/` directory. Run them with:

```sh
make test
```

This builds the test binary and exercises many of the library functions.

## License

This project is released under the terms of the
[GNU General Public License version 3](LICENSE).

