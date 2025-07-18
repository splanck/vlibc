[← Back to index](index.md)

## Utilities

Basic integer helpers cover simple absolute value and division routines.
`abs`, `labs` and `llabs` return the magnitude of an `int`, `long` or
`long long` argument. `div`, `ldiv` and `lldiv` compute both the quotient
and remainder of a division in a single call.

```c
int a = abs(-5);              // 5
ldiv_t r = ldiv(10L, 3L);     // r.quot == 3, r.rem == 1
```

## Regular Expressions

The `regex` module offers a lightweight matcher covering a small
subset of POSIX expressions.  Patterns are compiled with `regcomp`
and executed using `regexec`.

```c
regex_t re;
regcomp(&re, "h.*o", 0);
if (regexec(&re, "hello", 0, NULL, 0) == 0) {
    /* matched */
}
regfree(&re);
```

Supported features include:
`.` wildcard, `*`, `+`, `?`, and repetition with `{m,n}`, `{m}` or `{m,}`,
character classes `[]` (including POSIX forms like `[:digit:]`),
alternation using `|`, and anchors `^` and `$` which may appear
anywhere. Parentheses both group expressions and create numbered
capture groups which can be referenced via backreferences like
`\1` and `\2`.
Repetition ranges apply to the preceding token or group.

Example with a backreference:

```c
regex_t r;
regcomp(&r, "(foo)bar\\1", 0);
if (regexec(&r, "foobarfoo", 0, NULL, 0) == 0) {
    /* matches */
}
regfree(&r);
```

Another example using alternation and a repetition range:

```c
regex_t p;
regcomp(&p, "(foo|bar){2,3}", 0);
if (regexec(&p, "foobarfoo", 0, NULL, 0) == 0) {
    /* matches two or three occurrences of 'foo' or 'bar' */
}
regfree(&p);
```

### Limitations
- The engine performs simple backtracking and may be slow on
  pathological patterns.
- Nested alternations expand combinatorially and are intended only
  for small patterns.

## Math Functions

`sin`, `cos`, `tan`, `sqrt`, `pow`, `log`, `log2`, `log10`, `exp`, `exp2`,
`expm1`, `log1p`, `ldexp`, `floor`, `ceil`, `round`, `trunc`, `hypot`, `fmod`,
`fabs`, `fabsf`, `fmin`, `fmax`, `copysign`, `atan2`, `atan`, `asin`, `acos`,
`asinh`, `acosh`, `atanh`, `sinh`, `cosh`, and `tanh` are provided in
`math.h`. `float` and `long double` variants exist for the inverse and exponential helpers. These use simple series approximations and are suitable for basic calculations but may lack high precision.
`complex.h` declares simple complex routines `cabs`, `carg`, `cexp`, `ccos`
and `csin` built on the same helpers.

`math.h` also defines macros to classify floating-point values. `isnan(x)`
returns non-zero when `x` is not a number, `isinf(x)` detects positive or
negative infinity, and `isfinite(x)` reports true only for finite values.  When
the compiler supplies built-ins for these checks the macros map directly to
them, otherwise simple fallback tests are used.


## Suboption Parsing

`getsubopt` breaks down a comma separated list of `name=value` pairs. The
function compares each name against a provided token array and returns the
matching index. `*optionp` is updated to point past the parsed element and
`*valuep` receives the option's value if one was specified.

```c
char opts[] = "foo=1,bar,baz=2";
char *p = opts;
char *val;
char *tokens[] = {"foo", "bar", "baz", NULL};

while (*p) {
    switch (getsubopt(&p, tokens, &val)) {
    case 0:
        printf("foo=%s\n", val);
        break;
    case 1:
        puts("bar present");
        break;
    case 2:
        printf("baz=%s\n", val);
        break;
    default:
        printf("unknown option\n");
        break;
    }
}
```

## Hash Table Search

`hcreate`, `hdestroy` and `hsearch` implement a very small global hash table.
Create the table with `hcreate(nel)`, insert elements using `hsearch` with
`ENTER`, and look them up with `FIND`. `hdestroy` releases the table.

```c
hcreate(8);
ENTRY e = {"key", "value"};
hsearch(e, ENTER);
ENTRY q = {"key", NULL};
ENTRY *r = hsearch(q, FIND);
if (r)
    printf("%s\n", (char *)r->data);
hdestroy();
```

Only one table may exist at a time and collisions are resolved with linear
probing.

## Binary Search Trees

`tsearch`, `tfind`, `tdelete` and `twalk` build a minimal binary search tree.
Insert elements with `tsearch`, locate them using `tfind` and remove them with
`tdelete`. `twalk` visits each node in preorder, postorder and endorder.

```c
void *root = NULL;
int values[] = {4, 2, 7, 1, 6};
for (int i = 0; i < 5; i++)
    tsearch(&values[i], &root, int_cmp);
int *p = tfind(&values[2], &root, int_cmp);  // points to 7
int *parent = tdelete(&values[1], &root, int_cmp); // remove 2, parent->4
static int sum = 0;
void collect(const void *node, VISIT v, int l)
{
    if (v == postorder || v == leaf)
        sum += *(const int *)node;
}
twalk(root, collect);   // sum == 18
```

`tdelete` returns a pointer to the parent node's key (or a dangling pointer if
the removed node was the root).

## Message Formatting

`fmtmsg` prints a formatted diagnostic to standard error when `MM_PRINT` is
specified. The function accepts a classification mask, message label,
severity level, text, action hint and tag string.

```c
fmtmsg(MM_PRINT, "util:sub", MM_ERROR,
       "bad input", "retry with valid file", "UTIL:001");
```
