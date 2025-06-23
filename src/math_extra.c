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
