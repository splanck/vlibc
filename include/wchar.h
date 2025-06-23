#ifndef WCHAR_H
#define WCHAR_H

#include <stddef.h>
#include <stdarg.h>
#include "stdio.h"

#ifndef __wchar_t_defined
typedef int wchar_t;
#define __wchar_t_defined
#endif

typedef struct { int __dummy; } mbstate_t;

int mbtowc(wchar_t *pwc, const char *s, size_t n);
int wctomb(char *s, wchar_t wc);
size_t wcslen(const wchar_t *s);
int wcwidth(wchar_t wc);
int wcswidth(const wchar_t *s, size_t n);
size_t mbrtowc(wchar_t *pwc, const char *s, size_t n, mbstate_t *ps);
size_t wcrtomb(char *s, wchar_t wc, mbstate_t *ps);
size_t mbstowcs(wchar_t *dst, const char *src, size_t n);
size_t wcstombs(char *dst, const wchar_t *src, size_t n);
size_t mbrlen(const char *s, size_t n, mbstate_t *ps);
int mbsinit(const mbstate_t *ps);

/* Wide-character formatted output */
int wprintf(const wchar_t *format, ...);
int fwprintf(FILE *stream, const wchar_t *format, ...);
int swprintf(wchar_t *str, size_t size, const wchar_t *format, ...);
int vwprintf(const wchar_t *format, va_list ap);
int vfwprintf(FILE *stream, const wchar_t *format, va_list ap);
int vswprintf(wchar_t *str, size_t size, const wchar_t *format, va_list ap);

/* Wide-character formatted input */
int wscanf(const wchar_t *format, ...);
int fwscanf(FILE *stream, const wchar_t *format, ...);
int swscanf(const wchar_t *str, const wchar_t *format, ...);
int vwscanf(const wchar_t *format, va_list ap);
int vfwscanf(FILE *stream, const wchar_t *format, va_list ap);
int vswscanf(const wchar_t *str, const wchar_t *format, va_list ap);

#endif /* WCHAR_H */
