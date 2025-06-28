# vlibc

vlibc is a lightweight C library for UNIX-like systems. It provides just enough runtime support for small
programs and favors static linking so binaries remain self contained. The library targets Linux and the BSD
family.

## Features

- Thin wrappers for file I/O and filesystem utilities
- Process creation, threading and synchronization primitives
- Socket helpers and basic networking functions
- Small string, memory and math routines
- Locale, time and random number utilities

See the [documentation](docs/index.md) for the full API reference and examples.

## Building

```sh
make
make install PREFIX=/usr/local
```

## Example

```c
#include <stdio.h>

int main(void) {
    printf("hello, world\n");
    return 0;
}
```

Compile with:

```sh
cc -Iinclude hello.c libvlibc.a -o hello
```

## Tests

Run `make test` to execute the suite. Use `TEST_GROUP` to select a subset.

## License

Released under the [BSD 2-Clause "Simplified" License](LICENSE).
