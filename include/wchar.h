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
size_t mbrtowc(wchar_t *pwc, const char *s, size_t n, mbstate_t *ps);
size_t wcrtomb(char *s, wchar_t wc, mbstate_t *ps);
size_t mbstowcs(wchar_t *dst, const char *src, size_t n);
size_t wcstombs(char *dst, const wchar_t *src, size_t n);
size_t mbrlen(const char *s, size_t n, mbstate_t *ps);
int mbsinit(const mbstate_t *ps);

#endif /* WCHAR_H */
