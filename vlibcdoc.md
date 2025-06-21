# vlibc Documentation

This document outlines the architecture, planned modules, and API design for **vlibc**. The goal is to provide a clear overview for contributors and users who wish to understand how the library is organized.

## Table of Contents

1. [Initialization](#initialization)
2. [Memory Management](#memory-management)
3. [I/O](#io)
4. [String Handling](#string-handling)
5. [Process Control](#process-control)

## Architecture

vlibc aims to implement a minimal subset of the standard C library. The project is structured around small, easily auditable modules. Each module exposes a minimal API and relies on direct system calls whenever possible. The code is written in a straightforward C style without complex build systems.

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

The **memory** module implements a tiny allocator suitable for embedded or minimal environments. The design plans include:

- A wrapper for `brk`/`sbrk` to manage a contiguous heap region.
- Simple `vmalloc` and `vfree` functions with minimal overhead.
- Optional pooling of small blocks for reduced fragmentation.
- No support for advanced features like `mmap` by default, but hooks are provided for projects that require it.

The focus is on predictability and small code size rather than high performance.

## I/O

The **io** module exposes a limited set of I/O primitives:

- `vread` and `vwrite` for interacting with file descriptors.
- Thin wrappers around `open`, `close`, and `lseek` for basic file management.
- Helper functions for writing strings or buffers to standard output and standard error.

These functions map directly to system calls and avoid buffering or heavy state management.

## String Handling

The **string** module provides fundamental operations needed by most C programs:

- `vstrlen`, `vstrcpy`, and `vstrncmp` equivalents.
- Minimal locale or encoding support; all strings are treated as byte sequences.
- Utility functions for tokenizing and simple formatting.

The goal is to offer just enough functionality for common tasks without the complexity of full locale-aware libraries.

## Process Control

Process-related functionality resides in the **process** module. Planned features include:

- `vfork`, `vexecve`, and `vwaitpid` wrappers for spawning and tracking processes.
- Simplified environment handling for child processes.
- Basic signal support to allow clean termination and child process monitoring.

The design favors straightforward semantics over comprehensive POSIX conformance.

## Conclusion

vlibc is intentionally small and focused. This documentation will evolve as the project grows, but the guiding principles of minimalism and clarity will remain the same. Contributions are welcome as long as they align with these goals.

