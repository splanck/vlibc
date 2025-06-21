#ifndef STRING_H
#define STRING_H

#include <stddef.h>

size_t vstrlen(const char *s);
char *vstrcpy(char *dest, const char *src);
int vstrncmp(const char *s1, const char *s2, size_t n);

#endif /* STRING_H */
