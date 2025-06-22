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

See [docs/overview.md](docs/overview.md) for additional build notes and
[docs/architecture.md](docs/architecture.md) for design details. A complete
API reference is available in [vlibcdoc.md](vlibcdoc.md).

