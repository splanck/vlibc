#include "math.h"

static const double PI = 3.14159265358979323846;

static double fabs_approx(double x)
{
    return x < 0 ? -x : x;
}

double sin(double x)
{
    while (x > PI)
        x -= 2.0 * PI;
    while (x < -PI)
        x += 2.0 * PI;

    double term = x;
    double sum = x;
    for (int i = 1; i < 10; ++i) {
        term *= -x * x / ((2 * i) * (2 * i + 1));
        sum += term;
    }
    return sum;
}

double cos(double x)
{
    while (x > PI)
        x -= 2.0 * PI;
    while (x < -PI)
        x += 2.0 * PI;

    double term = 1.0;
    double sum = 1.0;
    for (int i = 1; i < 10; ++i) {
        term *= -x * x / ((2 * i - 1) * (2 * i));
        sum += term;
    }
    return sum;
}

double tan(double x)
{
    double s = sin(x);
    double c = cos(x);
    return c == 0.0 ? 0.0 : s / c;
}

double sqrt(double x)
{
    if (x <= 0.0)
        return 0.0;

    double guess = x;
    for (int i = 0; i < 20; ++i)
        guess = 0.5 * (guess + x / guess);
    return guess;
}

static double log_approx(double x)
{
    if (x <= 0.0)
        return 0.0;
    double y = (x - 1.0) / (x + 1.0);
    double y2 = y * y;
    double term = y;
    double sum = 0.0;
    for (int i = 1; i < 10; i += 2) {
        sum += term / i;
        term *= y2;
    }
    return 2.0 * sum;
}

static double exp_approx(double x)
{
    double term = 1.0;
    double sum = 1.0;
    for (int i = 1; i <= 10; ++i) {
        term *= x / i;
        sum += term;
    }
    return sum;
}

double pow(double base, double exp)
{
    if (exp == 0.0)
        return 1.0;
    if (exp < 0.0)
        return 1.0 / pow(base, -exp);

    int iexp = (int)exp;
    if (fabs_approx(exp - iexp) < 1e-9) {
        double result = 1.0;
        for (int i = 0; i < iexp; ++i)
            result *= base;
        return result;
    }

    return exp_approx(exp * log_approx(base));
}
