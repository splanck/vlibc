[← Back to index](index.md)

## Memory Management

The **memory** module provides a very small heap allocator implemented in
`memory.c`. When available it uses the `sbrk` system call to extend the heap.
On systems without `sbrk` the allocator falls back to `mmap`/`munmap` for
obtaining memory from the kernel. The implementation deliberately keeps things
simple. Each allocation stores a small header so the most recent block can be
released on `free()`. Memory for older blocks is not recycled when using
`sbrk`, keeping the code easy to audit at the cost of efficiency. When built
with the `mmap` backend each `free` call unmaps the region entirely. The free
list is protected by a mutex so all allocation functions are safe to call from
multiple threads.

### API

```c
void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
void *reallocf(void *ptr, size_t size);
void *reallocarray(void *ptr, size_t nmemb, size_t size);
void *recallocarray(void *ptr, size_t nmemb, size_t size);
int posix_memalign(void **memptr, size_t alignment, size_t size);
void *aligned_alloc(size_t alignment, size_t size);
```

### Behavior and Caveats

- When compiled with `HAVE_SBRK`, `malloc` allocates memory linearly using
  `sbrk` and returns `NULL` on failure. `free` only releases memory if called on
  the most recently allocated block.
- When `sbrk` is unavailable `malloc` obtains pages with `mmap` and `free`
  unmaps them with `munmap`.
- `calloc` calls `malloc` and zeroes the allocated block.
- `realloc` always allocates a new block and copies up to `size` bytes from the
  old pointer if one was provided.
- `reallocf` behaves like `realloc` but frees the original block when the
  resize fails.
- `reallocarray` multiplies `nmemb` and `size` with overflow checks, returning
  `NULL` and setting `errno` to `ENOMEM` on overflow.
- `recallocarray` behaves like `reallocarray` but zeroes any newly allocated
  region when the array grows.
- `posix_memalign` stores the allocated pointer in `*memptr` with the requested
  alignment. It returns `0` on success, `EINVAL` if the alignment is not a power
  of two or not a multiple of `sizeof(void *)`, or `ENOMEM` when the allocation
  fails.
- `aligned_alloc` wraps `posix_memalign` and returns `NULL` on failure.

```c
void *p;
if (posix_memalign(&p, 64, 128) == 0) {
    /* use memory */
    free(p);
}

void *q = aligned_alloc(64, 128);
if (q) {
    /* use memory */
    free(q);
}
```

Because memory is only reclaimed for the most recent block, applications that
allocate many objects may still eventually exhaust the heap. These routines are
sufficient for
small examples but should not be considered production quality.

## Memory Mapping

The `sys/mman.h` header exposes wrappers for interacting with the kernel's
memory mapping facilities. Available functions are:

```c
void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int munmap(void *addr, size_t length);
int mprotect(void *addr, size_t length, int prot);
int msync(void *addr, size_t length, int flags);
int mlock(const void *addr, size_t length);
int munlock(const void *addr, size_t length);
int mlockall(int flags);
int munlockall(void);
int madvise(void *addr, size_t length, int advice);
int posix_madvise(void *addr, size_t length, int advice);
```

`mmap` creates new mappings, `munmap` releases them and `mprotect` changes
their access protections.  `msync` flushes modified pages back to their
underlying file.  `mlock` and `munlock` lock or unlock specific regions,
while `mlockall` and `munlockall` operate on the entire address space.
`madvise` provides usage hints to the kernel.  When the raw syscall is
unavailable the BSD wrapper is used instead, so some platforms may ignore
unsupported flags.
`posix_madvise` offers the same interface but returns an error code
instead of setting `errno`.

## Shared Memory

`shm_open` creates or opens a named object in the system's shared memory
namespace. After sizing it with `ftruncate`, map the object with `mmap`
and unlink it when no longer needed:

```c
const char *name = "/example";
int fd = shm_open(name, O_CREAT | O_RDWR, 0600);
ftruncate(fd, 4096);
void *mem = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
                 MAP_SHARED, fd, 0);
/* use memory */
munmap(mem, 4096);
close(fd);
shm_unlink(name);
```

## POSIX Message Queues

Message queues allow processes to exchange fixed-size messages. For creating
new processes see [process.md](process.md). Create or
open a queue with `mq_open` and use `mq_send`/`mq_receive` to transfer
data:

```c
struct mq_attr attr = {0};
attr.mq_maxmsg = 4;
attr.mq_msgsize = 32;
mqd_t mq = mq_open("/example", O_CREAT | O_RDWR, 0600, &attr);
mq_send(mq, "hi", 3, 0);
char buf[32];
mq_receive(mq, buf, sizeof(buf), NULL);
mq_close(mq);
mq_unlink("/example");
```

