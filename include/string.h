#ifndef STRING_H
#define STRING_H

#include <stddef.h>

size_t vstrlen(const char *s);
char *vstrcpy(char *dest, const char *src);
int vstrncmp(const char *s1, const char *s2, size_t n);
int strcmp(const char *s1, const char *s2);
char *strchr(const char *s, int c);
char *strdup(const char *s);
char *strncpy(char *dest, const char *src, size_t n);
void *vmemcpy(void *dest, const void *src, size_t n);
void *vmemmove(void *dest, const void *src, size_t n);
void *vmemset(void *s, int c, size_t n);
int vmemcmp(const void *s1, const void *s2, size_t n);

void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
char *strtok(char *str, const char *delim);
char *strtok_r(char *str, const char *delim, char **saveptr);

#endif /* STRING_H */

#include_next <string.h>
