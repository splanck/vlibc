/*
 * BSD 2-Clause License
 *
 * Purpose: Declarations for basic math routines.
 */
#ifndef MATH_H
#define MATH_H

double sin(double x);
double cos(double x);
double tan(double x);
double sqrt(double x);
double pow(double base, double exp);
double log(double x);
double log10(double x);
double exp(double x);
double floor(double x);
double ceil(double x);
double fabs(double x);
double atan2(double y, double x);
double sinh(double x);
double cosh(double x);
double tanh(double x);

double fmod(double x, double y);
float fabsf(float x);
double ldexp(double x, int exp);
double log2(double x);
double fmin(double a, double b);
double fmax(double a, double b);
double copysign(double x, double y);

#endif /* MATH_H */
