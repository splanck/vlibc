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

Build the static library with:

```sh
make
```

Install the headers and library system-wide with:

```sh
make install PREFIX=/usr/local
```

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

For detailed documentation, see [vlibcdoc.md](vlibcdoc.md).

## Running Tests

Execute the unit test suite by running:

```sh
make test
```

This builds the test binary and runs all tests.

## License

This project is released under the terms of the
[GNU General Public License version 3](LICENSE).

