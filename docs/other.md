[← Back to index](index.md)

## Logging

`syslog.h` provides simple helpers to send log messages to `/dev/log` on
BSD-style systems. Call `openlog` once with an identifier then use `syslog`
with a priority and `printf`-style format:

```c
openlog("myapp", LOG_PID, LOG_USER);
syslog(LOG_INFO, "started with %d workers", workers);
closelog();
```

The underlying function `vsyslog` accepts a `va_list` so you can wrap the
logger with custom helpers:

```c
static void debug(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsyslog(LOG_DEBUG, fmt, ap);
    va_end(ap);
}
```

Messages are written using a Unix datagram socket so applications can integrate
with the host's syslog daemon.

## Raw System Calls

The `syscall` function from `syscall.h` invokes the host's `syscall()`
wrapper when available. Platform specific implementations may override
this if the operating system lacks a compatible interface.

## Non-local Jumps

When possible vlibc defers to the host C library's `setjmp` and `longjmp`.
For targets lacking a native implementation, custom versions live under
`src/arch/<arch>/setjmp.c`.

```c
int setjmp(jmp_buf env);
void longjmp(jmp_buf env, int val);
```

Jumping across signal handlers may leave blocked signals in an undefined
state.

## Floating-Point Environment

`<fenv.h>` exposes a small portion of the floating-point environment. Use
`fegetround` and `fesetround` to query or change the rounding mode and
`feclearexcept` to reset exception flags. The `fenv_t` type can store the
environment for later restoration with `fegetenv` and `fesetenv`.

```c
fenv_t env;
fegetenv(&env);
fesetround(FE_DOWNWARD);
double d = nearbyint(1.2); // rounds toward -INF => 1.0
fesetenv(&env);
```

## Termios Helpers

vlibc wraps standard terminal attribute routines.  Settings are read with
`tcgetattr()` and applied via `tcsetattr()`.  Queued data can be managed using
`tcdrain()`, `tcflow()`, `tcflush()` and `tcsendbreak()`.  Input and output
speeds stored in a `termios` structure are changed with `cfsetispeed()` and
`cfsetospeed()` or obtained with `cfgetispeed()` and `cfgetospeed()`.

## Limitations

 - The I/O routines perform simple optional buffering and provide only
   basic error reporting.
 - Process creation currently relies on Linux-specific syscalls.
 - BSD support is experimental and some subsystems may not compile yet.
 - The `system()` helper spawns `/bin/sh -c` and lacks detailed status
   codes.
 - `perror` and `strerror` support the full set of standard errno values.
- Thread support is limited to basic mutexes, condition variables,
  semaphores, barriers and join/detach.
- Named semaphores fall back to process-local objects on non-BSD systems and are
  not shareable across processes. For fork and exec helpers see
  [process.md](process.md).
- Locale handling falls back to the host implementation for values other
  than `"C"` or `"POSIX"`.
- `setjmp`/`longjmp` rely on the host C library when available.
  Only an x86_64 fallback implementation is provided.
- Regular expressions cover only a subset of POSIX syntax. Capture
  groups and numeric backreferences are supported but more advanced
  features remain unimplemented.
- The `crypt` helper only implements the traditional DES algorithm when
  BSD's extended `crypt` is unavailable.

## Conclusion

vlibc is intentionally small and focused. This documentation will evolve as the project grows, but the guiding principles of minimalism and clarity will remain the same. Contributions are welcome as long as they align with these goals.
