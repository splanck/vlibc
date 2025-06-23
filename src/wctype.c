#include "wctype.h"

int iswalpha(wint_t wc)
{
    if (wc < 0)
        return 0;
    if ((unsigned)wc < 0x80)
        return isalpha((unsigned char)wc);
    if ((wc >= 0xC0 && wc <= 0xD6) ||
        (wc >= 0xD8 && wc <= 0xF6) ||
        (wc >= 0xF8 && wc <= 0xFF))
        return 1;
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    extern int host_iswalpha(wint_t) __asm("iswalpha");
    return host_iswalpha(wc);
#else
    return 0;
#endif
}

int iswdigit(wint_t wc)
{
    return (wc >= L'0' && wc <= L'9');
}

int iswalnum(wint_t wc)
{
    return iswalpha(wc) || iswdigit(wc);
}

int iswspace(wint_t wc)
{
    switch (wc) {
    case L' ': case L'\t': case L'\n': case L'\r': case L'\v': case L'\f':
        return 1;
    case 0xA0:
        return 1; /* non-breaking space */
    default:
        return 0;
    }
}

int iswupper(wint_t wc)
{
    if (wc < 0)
        return 0;
    if ((unsigned)wc < 0x80)
        return isupper((unsigned char)wc);
    if ((wc >= 0xC0 && wc <= 0xD6) || (wc >= 0xD8 && wc <= 0xDE))
        return 1;
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    extern int host_iswupper(wint_t) __asm("iswupper");
    return host_iswupper(wc);
#else
    return 0;
#endif
}

int iswlower(wint_t wc)
{
    if (wc < 0)
        return 0;
    if ((unsigned)wc < 0x80)
        return islower((unsigned char)wc);
    if ((wc >= 0xDF && wc <= 0xF6) || (wc >= 0xF8 && wc <= 0xFF))
        return 1;
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    extern int host_iswlower(wint_t) __asm("iswlower");
    return host_iswlower(wc);
#else
    return 0;
#endif
}

int iswxdigit(wint_t wc)
{
    if (wc >= L'0' && wc <= L'9')
        return 1;
    if (wc >= L'a' && wc <= L'f')
        return 1;
    if (wc >= L'A' && wc <= L'F')
        return 1;
    return 0;
}

wint_t towlower(wint_t wc)
{
    if (iswupper(wc))
        return wc + (L'a' - L'A');
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    if ((unsigned)wc >= 0x80) {
        extern wint_t host_towlower(wint_t) __asm("towlower");
        return host_towlower(wc);
    }
#endif
    return wc;
}

wint_t towupper(wint_t wc)
{
    if (iswlower(wc))
        return wc - (L'a' - L'A');
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    if ((unsigned)wc >= 0x80) {
        extern wint_t host_towupper(wint_t) __asm("towupper");
        return host_towupper(wc);
    }
#endif
    return wc;
}

