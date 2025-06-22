# vlibc

A minimal libc replacement for UNIX-like systems.

The library includes wrappers for file I/O, memory management, processes,
threading, networking and other core functionality.

Build the static library with:

```sh
make
```

Install the headers and library system-wide with:

```sh
make install PREFIX=/usr/local
```

For a comprehensive guide covering build notes, architectural details and the
full API reference, see [vlibcdoc.md](vlibcdoc.md).

