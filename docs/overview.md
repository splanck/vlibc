[← Back to index](index.md)

## Overview

vlibc is a minimal libc replacement targeted at UNIX-like systems. It provides a
small subset of the standard C library with an emphasis on simplicity and
predictability. The project is distributed under the terms of the BSD 2-Clause
"Simplified" License.

### Goals
- Keep the code base small and auditable.
- Supply only the essential C runtime facilities.
- Offer consistent behavior for statically linked or embedded programs.

### Target Platforms
- Linux (x86_64, aarch64, armv7) and the major BSD variants (FreeBSD,
  OpenBSD, NetBSD) are supported.
- Other POSIX systems should work with minimal porting effort.
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

You can run a subset using the `TEST_GROUP` variable or convenience targets:

```sh
make test TEST_GROUP=memory
make test-memory
make test-network
```

This builds `tests/run_tests` and executes the suite.

