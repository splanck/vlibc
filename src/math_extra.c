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

#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
extern double host_fmod(double, double) __asm("fmod");
extern float host_fabsf(float) __asm("fabsf");
extern double host_ldexp(double, int) __asm("ldexp");
extern double host_log2(double) __asm("log2");
extern double host_fmin(double, double) __asm("fmin");
extern double host_fmax(double, double) __asm("fmax");
extern double host_copysign(double, double) __asm("copysign");
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
