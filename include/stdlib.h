/*
 * BSD 2-Clause License
 *
 * Purpose: Declarations for basic utilities.
 */
#ifndef STDLIB_H
#define STDLIB_H

#include <stddef.h>

/* Environment variable access */
extern char **environ;
char *getenv(const char *name);
int setenv(const char *name, const char *value, int overwrite);
int unsetenv(const char *name);

/* Current working directory */
char *getcwd(char *buf, size_t size);
char *realpath(const char *path, char *resolved_path);

/* Path utilities */
char *basename(const char *path);
char *dirname(const char *path);
char *mkdtemp(char *template);

/* Execute a shell command */
int system(const char *command);

/* String conversion */
long strtol(const char *nptr, char **endptr, int base);
unsigned long strtoul(const char *nptr, char **endptr, int base);
long long strtoll(const char *nptr, char **endptr, int base);
unsigned long long strtoull(const char *nptr, char **endptr, int base);
int atoi(const char *nptr);
double strtod(const char *nptr, char **endptr);
double atof(const char *nptr);


/* Sorting helpers */
void qsort(void *base, size_t nmemb, size_t size,
           int (*compar)(const void *, const void *));
void qsort_r(void *base, size_t nmemb, size_t size,
             int (*compar)(const void *, const void *, void *), void *ctx);
void *bsearch(const void *key, const void *base, size_t nmemb, size_t size,
              int (*compar)(const void *, const void *));

/* Pseudo-random number generator */
int rand(void);
void srand(unsigned seed);
unsigned int arc4random(void);
void arc4random_buf(void *buf, size_t len);
int rand_r(unsigned *state);

/* Register a function to run at normal process exit */
int atexit(void (*fn)(void));

/* Terminate the process with SIGABRT */
void abort(void);

#endif /* STDLIB_H */
