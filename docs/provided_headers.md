[← Back to index](index.md)

## Provided Headers

vlibc installs a small set of public headers for application use:

```c
ctype.h      - character classification helpers
dirent.h     - directory iteration
dlfcn.h      - runtime loading of shared libraries
env.h        - environment variable access
errno.h      - standard error codes
err.h        - err/warn convenience helpers
getopt.h     - option parsing
io.h         - unbuffered I/O primitives
locale.h     - locale helpers
math.h       - basic math routines
complex.h   - basic complex math routines
memory.h     - heap allocation
assert.h     - runtime assertion checks
netdb.h      - address resolution helpers
arpa/inet.h  - IPv4/IPv6 presentation conversion helpers (`inet_pton`,
                `inet_ntop`, `inet_aton`, `inet_ntoa`)
netinet/in.h - network byte order helpers
ftw.h        - directory tree traversal helpers
fts.h        - file tree walk helpers
poll.h       - I/O multiplexing helpers
signal.h    - signal handling helpers
process.h    - process creation and control
pthread.h    - minimal threading support
setjmp.h     - non-local jump helpers
stdio.h      - simple stream I/O
stdlib.h     - basic utilities
string.h     - string manipulation
iconv.h      - character set conversion helpers
regex.h     - simple regular expression matching
sched.h     - processor yield helper
termios.h   - terminal attribute helpers
unistd.h    - POSIX I/O and process helpers
sys/file.h   - file permission helpers
sys/mman.h   - memory mapping helpers
sys/select.h - fd_set macros and select wrapper
sys/socket.h - networking wrappers
sys/stat.h   - file status functions
sys/ipc.h    - System-V IPC keys and permissions
sys/msg.h    - System-V message queues
sys/sem.h    - System-V semaphores
sys/shm.h    - System-V shared memory and POSIX helpers
sys/statvfs.h - filesystem statistics
syscall.h    - raw syscall interface
time.h       - time related helpers
vlibc.h      - library initialization
wchar.h      - wide character helpers
wctype.h     - wide character classification helpers
```


