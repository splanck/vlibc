[← Back to index](index.md)

## String Handling

The **string** module provides fundamental operations needed by most C programs:

- `vstrlen`, `vstrcpy`, `vstrncmp`, `strnlen`, `strcat`, `strncat`, `strlcpy`, `strlcat`, `stpcpy` and `stpncpy` equivalents.
- `strdup` and `strndup` helpers allocate new copies of strings.
- Search helpers `strstr`, `strrchr`, `memchr`, `memrchr`, and `memmem` for locating substrings or bytes.
- Wide-character search helpers `wcschr`, `wcsrchr`, `wcsstr` and `wmemchr` mirror those operations for `wchar_t` data.
- Prefix scanners `strspn` and `strcspn` along with `strpbrk` for finding any character from a set.
- Case-insensitive comparisons `strcasecmp` and `strncasecmp`.
- Case-insensitive substring search with `strcasestr`.
- Basic collation helpers `strcoll` and `strxfrm` act on ASCII strings. On
  BSD systems they defer to the host implementations when the active locale is
  not `"C"` or `"POSIX"`. The wide-character versions `wcscoll` and `wcsxfrm` behave the same way.
- Conventional memory routines (`memcpy`, `memmove`, `memset`, `memcmp`) map to
  the internal `v` implementations.
- `memccpy` stops copying when a byte value is found and returns a pointer past
  it. `mempcpy` copies `n` bytes and returns the destination end pointer.
- The low-level memory helpers `vmemcpy`, `vmemmove`, `vmemset`, and `vmemcmp` operate on raw byte buffers. `vmemcpy` copies bytes from a source to a destination, `vmemmove` handles overlaps safely, `vmemset` fills a region with a byte value, and `vmemcmp` compares two buffers. The standard `memcpy`, `memmove`, `memset`, and `memcmp` functions simply call these implementations.
- Basic locale handling reads the `LC_ALL` and `LANG` environment variables.
  `setlocale` defaults to those values and, on BSD systems, falls back to the
  host `setlocale(3)` when a locale other than `"C"` or `"POSIX"` is
  requested. `localeconv` exposes formatting data and `nl_langinfo` queries
  locale items such as `CODESET`. All strings are treated as byte sequences.
- Utility functions for tokenizing and simple formatting.
- `printf` style routines understand `%d`, `%u`, `%s`, `%x`, `%X`, `%o`, `%p`,
  and `%c` with basic field width and precision handling.
- `scanf` style routines parse `%d`, `%u`, `%x`, `%o`, `%s`, and floating
  point formats such as `%f`, `%lf`, and `%g`.
- `strtok` and `strtok_r` split a string into tokens based on a set of
  delimiter characters. `strtok` stores its parsing state in static
  memory and is not thread-safe. `strtok_r` lets the caller maintain the
  context and is safe for concurrent use.
  - Simple number conversion helpers `atoi`, `strtol`, `strtoul`, `strtoll`,
    `strtoull`, `strtoimax`, `strtoumax`, `strtof`, `strtod`, `strtold`, and `atof`.

### Example

```c
const char *text = "hello world";
size_t first_word = strcspn(text, " ");
char *vowel = strpbrk(text, "aeiou");
size_t prefix = strspn("abc123", "abc");
char dest[8];
char *end = memccpy(dest, "stop!", '!', 5);
end = mempcpy(dest, "end", 3);
```

Basic time formatting is available via `strftime` and the matching
`strptime` parser. `strftime` handles common conversions like
`%Y`, `%m`, `%d`, `%H`, `%M`, `%S`, `%a`, `%b`, `%Z`, `%z`, and weekday
numbers (`%w`/`%u`). The parser continues to accept the numeric
fields (`%Y`, `%m`, `%d`, `%H`, `%M`, `%S`). The output uses the current
locale. Non-`"C"` locales work when the host `setlocale(3)` accepts them
(primarily on BSD systems).

The library also includes simple conversion routines `gmtime`, `localtime`,
`mktime`, and `ctime`. They convert between `time_t` and `struct tm` or
produce a readable string. `localtime` applies the offset configured via
`tzset()` so results follow the `TZ` environment variable or `/etc/localtime`.

The goal is to offer just enough functionality for common tasks without the complexity of full locale-aware libraries.

### Wide Character Conversion

`mbtowc` converts a multibyte sequence to a single `wchar_t` and `wctomb`
performs the opposite conversion.  `mbrtowc`/`wcrtomb` are stateful
variants used by `mbstowcs` and `wcstombs` for converting entire
strings.  These helpers handle ASCII directly and fall back to the host
C library when encountering non-ASCII data.  `wcslen` returns the length
of a wide string excluding the terminator.
`mbsrtowcs` and `wcsrtombs` expose the same conversions while updating
the provided state pointer so that a string can be processed in
multiple steps.

`wcscpy`, `wcsncpy`, `wcscmp`, `wcsncmp`, and `wcsdup` mirror the
behaviour of their narrow-string counterparts for copying, comparing and
duplicating wide strings. Collation helpers `wcscoll` and `wcsxfrm` use
simple lexicographic ordering in the `C` locale and delegate to the host
implementation on BSD when other locales are active. The `wcstok`
function tokenizes a wide string using a caller-supplied save pointer.
The `wmemcpy`, `wmemmove`, `wmemset` and `wmemcmp` routines operate on
arrays of `wchar_t` analogous to the byte-oriented routines.
Search helpers `wcschr`, `wcsrchr`, `wcsstr` and `wmemchr` provide character
and substring lookup for wide strings.
Number parsing functions `wcstol`, `wcstoul`, `wcstoll`, `wcstoull`,
`wcstoimax`, `wcstoumax`, `wcstof`, `wcstod` and `wcstold` mirror the
behaviour of the narrow-string conversions.

### Example

```c
wchar_t src[] = { L'a', L'b', L'c' };
wchar_t dst[3];
wmemcpy(dst, src, 3);      // copy three wide characters
wmemmove(dst + 1, dst, 2); // move with overlap
wmemset(dst, L'X', 2);     // fill first two entries
int diff = wmemcmp(dst, src, 3);
```

Wide strings can be tokenized incrementally:

```c
wchar_t text[] = L"one two";
wchar_t *save = NULL;
wchar_t *t1 = wcstok(text, L" ", &save); // L"one"
wchar_t *t2 = wcstok(NULL, L" ", &save); // L"two"
```

`wcwidth` reports the number of columns needed to display a single wide
character while `wcswidth` sums the widths of up to `n` characters. ASCII
codepoints have the expected width and on BSD systems non-ASCII values
are delegated to the host implementation.

### Example

```c
int c = wcwidth(L'A');               // 1
int w = wcswidth(L"hello", 5);      // 5
```

### Single-Byte Conversions

`mblen` returns the number of bytes forming the next multibyte
character or `-1` when the sequence is invalid. ASCII input is handled directly and on BSD systems other locales are passed to the host C library. `btowc` converts a
single byte to a `wchar_t` using `mbrtowc` and yields `-1` on failure.
`wctob` performs the opposite operation through `wcrtomb`, returning the
resulting byte or `-1` when the character cannot be represented.

```c
int n = mblen("A", 1);       // 1
wint_t wc = btowc('A');     // L'A'
int c2 = wctob(L'A');       // 'A'
```

### Wide-Character I/O

`fgetwc` and `fputwc` mirror `fgetc` and `fputc` but operate on wide
characters. The `getwc` and `putwc` wrappers simply call these
functions. On failure they return `WEOF` and set `errno`. Passing a
`NULL` stream sets `EINVAL` while conversion errors use `EILSEQ`.

```c
FILE *fp = tmpfile();
fputwc(L'Z', fp);
rewind(fp);
wchar_t ch = fgetwc(fp);
// ch == L'Z'
fclose(fp);
```

### Wide Memory Streams

`open_wmemstream` returns a stream that stores its output as a dynamically
allocated wide string. Data written with `fwprintf` or `fputwc` grows the
buffer automatically and `fclose` exposes the resulting `wchar_t` array and
character count.

```c
wchar_t *out = NULL;
size_t len = 0;
FILE *ws = open_wmemstream(&out, &len);
fwprintf(ws, L"%ls %d", L"wide", 42);
fclose(ws);
// out -> L"wide 42", len -> 7
free(out);
```

### Character Set Conversion

`iconv_open` returns a descriptor for translating between character
sets.  vlibc understands only conversions between `"ASCII"` and
`"UTF-8"`.  The `iconv` function copies bytes from the input buffer to
the output buffer and fails with `EILSEQ` on bytes that cannot be
represented.  On BSD systems other conversions are delegated to the
host `iconv` implementation when present.

## Character Classification

Character checks live in [include/ctype.h](include/ctype.h).  The macros
operate on a small table of 128 entries, so only the standard ASCII
characters are recognized.

Wide-character classification is provided in [include/wctype.h](include/wctype.h).
The functions there test a few basic Unicode ranges and fall back to the host
implementation on BSD systems when available.

- `isalpha(c)`  – non-zero for letters `A`–`Z` or `a`–`z`.
- `isdigit(c)`  – non-zero for decimal digits `0`–`9`.
- `isalnum(c)`  – true when `isalpha(c)` or `isdigit(c)` is true.
- `isspace(c)`  – tests for whitespace characters such as space or tab.
- `isupper(c)`  – non-zero for uppercase letters.
- `islower(c)`  – non-zero for lowercase letters.
- `isxdigit(c)` – true for hexadecimal digits.
- `isprint(c)` – non-zero for characters with visible representation.
- `iscntrl(c)` – true for control characters like `\n`.
- `ispunct(c)` – true for punctuation characters.
- `isgraph(c)` – like `isprint(c)` but excludes space.
- `isblank(c)` – true for space or tab.
- `tolower(c)`  – converts an uppercase letter to lowercase.
- `toupper(c)`  – converts a lowercase letter to uppercase.

Values outside the ASCII range always fail the classification tests and are
returned unchanged by `tolower` and `toupper`.

## Option Parsing

Command-line arguments can be processed with `getopt`, `getopt_long` or
`getopt_long_only`. The former handles short options while the latter
two accept an array of `struct option` describing long names. The
`getopt_long_only` variant also recognizes long options when they are
prefixed with a single dash. Both long parsing functions return the
option's value field or set a flag when supplied in the table.


## Monetary Formatting

`strfmon` converts floating point values to formatted currency strings.
Only the `"C"` locale is supported, so the dollar sign is used as the
currency symbol. Width and precision specifiers are honored but
advanced flags are ignored.

```c
char out[32];
strfmon(out, sizeof(out), "%8.2n", 12.3); // "   $12.30"
```
