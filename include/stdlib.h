#ifndef STDLIB_H
#define STDLIB_H

#include <stddef.h>

/* Environment variable access */
extern char **environ;
char *getenv(const char *name);
int setenv(const char *name, const char *value, int overwrite);
int unsetenv(const char *name);

/* Execute a shell command */
int system(const char *command);

/* String conversion */
long strtol(const char *nptr, char **endptr, int base);
int atoi(const char *nptr);


/* Sorting helpers */
void qsort(void *base, size_t nmemb, size_t size,
           int (*compar)(const void *, const void *));
void *bsearch(const void *key, const void *base, size_t nmemb, size_t size,
              int (*compar)(const void *, const void *));

/* Pseudo-random number generator */
int rand(void);
void srand(unsigned seed);

#endif /* STDLIB_H */
