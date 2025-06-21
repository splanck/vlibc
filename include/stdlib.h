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

/* Pseudo-random number generator */
int rand(void);
void srand(unsigned seed);

#endif /* STDLIB_H */
