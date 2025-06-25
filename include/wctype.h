/*
 * BSD 2-Clause License
 *
 * Purpose: Declarations for wide character classification helpers.
 */
#ifndef WCTYPE_H
#define WCTYPE_H

#include "wchar.h"
#include "ctype.h"

#ifndef __wint_t_defined
typedef int wint_t;
#define __wint_t_defined
#endif

typedef int wctype_t;

/* Test for alphabetic wide character */
int iswalpha(wint_t wc);
/* Test for decimal digit wide character */
int iswdigit(wint_t wc);
/* Test for alphanumeric wide character */
int iswalnum(wint_t wc);
/* Test for whitespace wide character */
int iswspace(wint_t wc);
/* Test for uppercase wide character */
int iswupper(wint_t wc);
/* Test for lowercase wide character */
int iswlower(wint_t wc);
/* Test for hexadecimal digit wide character */
int iswxdigit(wint_t wc);
/* Convert wide character to lowercase */
wint_t towlower(wint_t wc);
/* Convert wide character to uppercase */
wint_t towupper(wint_t wc);

#endif /* WCTYPE_H */
