#ifndef WCTYPE_H
#define WCTYPE_H

#include "wchar.h"
#include "ctype.h"

#ifndef __wint_t_defined
typedef int wint_t;
#define __wint_t_defined
#endif

typedef int wctype_t;

int iswalpha(wint_t wc);
int iswdigit(wint_t wc);
int iswalnum(wint_t wc);
int iswspace(wint_t wc);
int iswupper(wint_t wc);
int iswlower(wint_t wc);
int iswxdigit(wint_t wc);
wint_t towlower(wint_t wc);
wint_t towupper(wint_t wc);

#endif /* WCTYPE_H */
