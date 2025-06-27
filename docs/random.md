[← Back to index](index.md)

## Random Numbers

vlibc provides a minimal pseudo-random number generator implemented as a
linear congruential generator.

```c
int rand(void);
void srand(unsigned seed);
unsigned int arc4random(void);
void arc4random_buf(void *buf, size_t len);
unsigned int arc4random_uniform(unsigned int upper_bound);
int rand_r(unsigned *state);
```

Calling `srand()` initializes the internal state. Reusing the same seed
produces the identical sequence of numbers, each in the range `0` to
`32767`.

`arc4random()` returns a 32-bit value sourced from the operating system's
random generator. `arc4random_buf()` fills an arbitrary buffer with secure
random bytes. At runtime vlibc first attempts the `getrandom(2)` system
call or a native `arc4random` implementation when available. If those
sources fail, `/dev/urandom` is consulted and as a final fallback the
buffer is populated using the `rand()` PRNG which offers little entropy.
`arc4random_uniform()` converts the 32-bit output into a value in the range
`[0, upper_bound)` using rejection sampling so each result occurs with
equal probability. The `rand_r()` variant operates like `rand()` but stores
its state in a user-provided variable so it can be used in threaded code.

## rand48 API

The `rand48` functions implement a 48-bit linear congruential generator
compatible with POSIX.  They share a global state initialized with
`srand48()`:

```c
double drand48(void);
double erand48(unsigned short state[3]);
long lrand48(void);
long nrand48(unsigned short state[3]);
void srand48(long seedval);
unsigned short *seed48(unsigned short state[3]);
void lcong48(unsigned short param[7]);
```

`drand48()` returns a floating-point value in the range `[0.0,1.0)`.
`lrand48()` produces a non-negative long integer.  The `erand48()` and
`nrand48()` variants operate on a caller supplied state array instead of
the global seed.  Calling `seed48()` swaps the generator state and
`lcong48()` allows customizing the multiplier and additive constants.

## Sorting Helpers

`qsort` sorts an array in place using a user-supplied comparison
function while `bsearch` performs binary search on a sorted array.
`qsort_r` acts like `qsort` but forwards a caller provided context
pointer to the comparison callback.

```c
int values[] = {4, 2, 7};
qsort(values, 3, sizeof(int), cmp_int);
int key = 7;
int *found = bsearch(&key, values, 3, sizeof(int), cmp_int);
```

`qsort_r` accepts an extra context pointer which is passed to the
comparison function. This can be used to change the sort order
dynamically:

```c
static int cmp_dir(const void *a, const void *b, void *ctx)
{
    int dir = *(int *)ctx;        // 1 for ascending, -1 for descending
    int x = *(const int *)a;
    int y = *(const int *)b;
    return dir * ((x > y) - (x < y));
}

int desc = -1;
qsort_r(values, 3, sizeof(int), cmp_dir, &desc);
```

