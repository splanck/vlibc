# vlibc Documentation

vlibc is a lightweight C library aimed at UNIX-like systems.  It provides only
the essential runtime helpers so small programs can be statically linked
without dragging in a full featured libc.  For usage examples see the
[README](../README.md).

## Documentation

| Topic | Description |
|-------|-------------|
| [Overview](overview.md) | Project goals and guiding principles |
| [Architecture](architecture.md) | Source layout and planned modules |
| [Provided Headers](provided_headers.md) | Public header files installed by vlibc |
| [Memory](memory.md) | Heap allocator, memory mapping and shared memory |
| [Process Control](process.md) | Fork, exec, signals and basic threading |
| [File I/O](io.md) | System call wrappers for file descriptors |
| [Networking](network.md) | Socket helpers and address utilities |
| [Filesystem](filesystem.md) | Permissions, directory walking and path helpers |
| [Strings](strings.md) | Common string manipulation functions |
| [Random Numbers](random.md) | Pseudo-random number generation |
| [Utilities](utilities.md) | Miscellaneous helpers such as `abs` and `div` |
| [Users and Groups](users_groups.md) | Access to system user and group databases |
| [Time](time.md) | Formatting, timers and resource usage |
| [Other Topics](other.md) | Logging and assorted functionality |
