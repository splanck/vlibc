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

## Limitations

- The I/O routines (`open`, `read`, `write`, `close`) are thin wrappers around
  the corresponding system calls. They perform no buffering and provide only
  basic error reporting.
- Process creation and signal functions rely on Linux `fork`, `execve`,
  `wait4`/`waitpid`, and `rt_sigaction` syscalls. Porting to other UNIX-like
  kernels may require adapting these calls.
