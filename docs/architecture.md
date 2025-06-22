# Architecture

vlibc aims to implement a minimal subset of the standard C library. The project is structured around small, easily auditable modules. Each module exposes a minimal API and relies on direct system calls whenever possible. The code is written in a straightforward C style without complex build systems.

The overall repository layout is described in the README. In short, source code
lives in `src/`, public headers in `include/`, and tests will be placed in
`tests/`.

### Planned Modules

- **startup**: Program entry, stack setup, and initialization routines.
- **memory**: Basic heap management using `brk`/`sbrk` or a small memory allocator.
- **io**: Simple wrappers around read/write system calls for file descriptors.
- **string**: Common string operations such as length checks and copying.
- **process**: Functions for spawning and waiting on child processes.
- **math**: Elementary math routines like `sin` and `sqrt`.

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

