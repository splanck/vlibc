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

/* Execute a shell command */
int system(const char *command);

/* String conversion */
long strtol(const char *nptr, char **endptr, int base);
int atoi(const char *nptr);
double strtod(const char *nptr, char **endptr);
double atof(const char *nptr);


/* Sorting helpers */
void qsort(void *base, size_t nmemb, size_t size,
           int (*compar)(const void *, const void *));
void *bsearch(const void *key, const void *base, size_t nmemb, size_t size,
              int (*compar)(const void *, const void *));

/* Pseudo-random number generator */
int rand(void);
void srand(unsigned seed);

/* Register a function to run at normal process exit */
int atexit(void (*fn)(void));

/* Terminate the process with SIGABRT */
void abort(void);

#endif /* STDLIB_H */
