#ifndef WCHAR_H
#define WCHAR_H

#include <stddef.h>

#ifndef __wchar_t_defined
typedef int wchar_t;
#define __wchar_t_defined
#endif

typedef struct { int __dummy; } mbstate_t;

int mbtowc(wchar_t *pwc, const char *s, size_t n);
int wctomb(char *s, wchar_t wc);
size_t wcslen(const wchar_t *s);

#endif /* WCHAR_H */
