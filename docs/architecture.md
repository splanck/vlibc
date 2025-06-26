[← Back to index](index.md)

## Architecture

vlibc is structured around small, easily auditable modules. Each module exposes a
minimal API and relies on direct system calls whenever possible. Source files
live in `src/`, public headers in `include/`, and tests reside in `tests/`.

### Planned Modules
- **startup**: Program entry, stack setup and initialization routines.
- **memory**: Basic heap management using `brk`/`sbrk` with a fallback to
  `mmap`/`munmap` when those calls are unavailable.
- **io**: Simple wrappers around read/write system calls for file descriptors.
- **string**: Common string operations such as length checks and copying.
- **process**: Functions for spawning and waiting on child processes.
- **math**: Elementary math routines like `sin` and `sqrt` plus hyperbolic
  helpers and `atan2`.

### API Design
vlibc strives for a consistent API surface inspired by the C standard library.
Functions are prefixed with `v` (e.g., `vmalloc`) to differentiate them from
their libc counterparts. Where appropriate, functions mirror POSIX signatures so
users can easily migrate existing code.

### Naming Conventions
- `v` prefix for all exported functions.
- Lowercase with underscores for function names and variables.
- Keep parameter lists short and intuitive.

### Initialization
Initialization code lives in the **startup** module and performs:

1. Setting up the runtime stack and registers.
2. Parsing arguments and environment variables.
3. Calling `v_main()` as the program entry point.
4. Providing a minimal exit routine to flush data and terminate.

Programs using vlibc define `v_main(argc, argv, envp)` to keep the startup
routine simple and avoid complex features like C++ static constructors.

