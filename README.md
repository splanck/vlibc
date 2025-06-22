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

For detailed documentation, see [vlibcdoc.md](vlibcdoc.md).

