/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms, with or without modification, are permitted provided that the copyright notice and this permission notice appear in all copies. This software is provided "as is" without warranty.
 *
 * Purpose: Implements the math_extra functions for vlibc. Provides wrappers and helpers used by the standard library.
 *
 * Copyright (c) 2025
 */

#include "math.h"

static const double PI = 3.14159265358979323846;

static double atan_series(double z)
{
    double term = z;
    double sum = z;
    for (int i = 1; i < 10; ++i) {
        term *= -z * z;
        sum += term / (2 * i + 1);
    }
    return sum;
}

static double atan_approx(double z)
{
    int sign = 1;
    if (z < 0.0) {
        sign = -1;
        z = -z;
    }

    double result;
    if (z > 1.0)
        result = PI / 2.0 - atan_series(1.0 / z);
    else
        result = atan_series(z);

    return sign * result;
}

double atan2(double y, double x)
{
    if (x > 0.0)
        return atan_approx(y / x);
    if (x < 0.0) {
        if (y >= 0.0)
            return atan_approx(y / x) + PI;
        else
            return atan_approx(y / x) - PI;
    }
    if (y > 0.0)
        return PI / 2.0;
    if (y < 0.0)
        return -PI / 2.0;
    return 0.0;
}

double log10(double x)
{
    static const double LN10 = 2.30258509299404568402;
    return log(x) / LN10;
}

double sinh(double x)
{
    double ex = exp(x);
    double em = exp(-x);
    return 0.5 * (ex - em);
}

double cosh(double x)
{
    double ex = exp(x);
    double em = exp(-x);
    return 0.5 * (ex + em);
}

double tanh(double x)
{
    double s = sinh(x);
    double c = cosh(x);
    return c == 0.0 ? 0.0 : s / c;
}

double atan(double x)
{
    return atan_approx(x);
}

double asin(double x)
{
    return atan2(x, sqrt(1.0 - x * x));
}

double acos(double x)
{
    return PI / 2.0 - asin(x);
}

double asinh(double x)
{
    return log(x + sqrt(x * x + 1.0));
}

double acosh(double x)
{
    return log(x + sqrt((x - 1.0) * (x + 1.0)));
}

double atanh(double x)
{
    return 0.5 * log((1.0 + x) / (1.0 - x));
}

double exp2(double x)
{
    return pow(2.0, x);
}

double expm1(double x)
{
    return exp(x) - 1.0;
}

double log1p(double x)
{
    return log(1.0 + x);
}

float asinf(float x)
{
    return (float)asin((double)x);
}

float acosf(float x)
{
    return (float)acos((double)x);
}

float atanf(float x)
{
    return (float)atan((double)x);
}

float asinhf(float x)
{
    return (float)asinh((double)x);
}

float acoshf(float x)
{
    return (float)acosh((double)x);
}

float atanhf(float x)
{
    return (float)atanh((double)x);
}

float exp2f(float x)
{
    return (float)exp2((double)x);
}

float expm1f(float x)
{
    return (float)expm1((double)x);
}

float log1pf(float x)
{
    return (float)log1p((double)x);
}

long double asinl(long double x)
{
    return (long double)asin((double)x);
}

long double acosl(long double x)
{
    return (long double)acos((double)x);
}

long double atanl(long double x)
{
    return (long double)atan((double)x);
}

long double asinhl(long double x)
{
    return (long double)asinh((double)x);
}

long double acoshl(long double x)
{
    return (long double)acosh((double)x);
}

long double atanhl(long double x)
{
    return (long double)atanh((double)x);
}

long double exp2l(long double x)
{
    return (long double)exp2((double)x);
}

long double expm1l(long double x)
{
    return (long double)expm1((double)x);
}

long double log1pl(long double x)
{
    return (long double)log1p((double)x);
}

#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
extern double host_fmod(double, double) __asm("fmod");
extern float host_fabsf(float) __asm("fabsf");
extern double host_ldexp(double, int) __asm("ldexp");
extern double host_log2(double) __asm("log2");
extern double host_fmin(double, double) __asm("fmin");
extern double host_fmax(double, double) __asm("fmax");
extern double host_copysign(double, double) __asm("copysign");
extern double host_hypot(double, double) __asm("hypot");
extern double host_round(double) __asm("round");
extern double host_trunc(double) __asm("trunc");
#endif

double fmod(double x, double y)
{
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    return host_fmod(x, y);
#else
    if (y == 0.0)
        return 0.0;
    long q = (long)(x / y);
    double r = x - (double)q * y;
    if (r < 0.0)
        r += y;
    return r;
#endif
}

float fabsf(float x)
{
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    return host_fabsf(x);
#else
    return x < 0.0f ? -x : x;
#endif
}

double ldexp(double x, int exp)
{
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    return host_ldexp(x, exp);
#else
    if (exp >= 0) {
        for (int i = 0; i < exp; ++i)
            x *= 2.0;
    } else {
        for (int i = 0; i < -exp; ++i)
            x *= 0.5;
    }
    return x;
#endif
}

double log2(double x)
{
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    return host_log2(x);
#else
    static const double LN2 = 0.69314718055994530942;
    return log(x) / LN2;
#endif
}

double fmin(double a, double b)
{
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    return host_fmin(a, b);
#else
    return (a < b) ? a : b;
#endif
}

double fmax(double a, double b)
{
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    return host_fmax(a, b);
#else
    return (a > b) ? a : b;
#endif
}

double copysign(double x, double y)
{
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    return host_copysign(x, y);
#else
    if ((y < 0 && x > 0) || (y >= 0 && x < 0))
        x = -x;
    return x;
#endif
}

double hypot(double x, double y)
{
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    return host_hypot(x, y);
#else
    return sqrt(x * x + y * y);
#endif
}

double round(double x)
{
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    return host_round(x);
#else
    double i = floor(x);
    double frac = x - i;
    if (x >= 0.0) {
        if (frac >= 0.5)
            i += 1.0;
    } else {
        if (frac <= -0.5)
            i -= 1.0;
    }
    return i;
#endif
}

double trunc(double x)
{
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    return host_trunc(x);
#else
    return (x >= 0.0) ? floor(x) : ceil(x);
#endif
}
