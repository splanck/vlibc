#ifndef STRING_H
#define STRING_H

#include <stddef.h>

#if defined(__has_include)
#  if __has_include("/usr/include/string.h")
#    include "/usr/include/string.h"
#  endif
#endif

size_t vstrlen(const char *s);
size_t strnlen(const char *s, size_t maxlen);
char *vstrcpy(char *dest, const char *src);
int vstrncmp(const char *s1, const char *s2, size_t n);
int strcmp(const char *s1, const char *s2);
char *strchr(const char *s, int c);
char *strdup(const char *s);
char *strncpy(char *dest, const char *src, size_t n);
char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, size_t n);
size_t strlcpy(char *dst, const char *src, size_t size);
size_t strlcat(char *dst, const char *src, size_t size);
void *vmemcpy(void *dest, const void *src, size_t n);
void *vmemmove(void *dest, const void *src, size_t n);
void *vmemset(void *s, int c, size_t n);
int vmemcmp(const void *s1, const void *s2, size_t n);

void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
void *memchr(const void *s, int c, size_t n);
size_t strspn(const char *s, const char *accept);
size_t strcspn(const char *s, const char *reject);
char *strpbrk(const char *s, const char *accept);
char *strrchr(const char *s, int c);
char *strstr(const char *haystack, const char *needle);
char *strtok(char *str, const char *delim);
char *strtok_r(char *str, const char *delim, char **saveptr);
int strcasecmp(const char *s1, const char *s2);
int strncasecmp(const char *s1, const char *s2, size_t n);

#endif /* STRING_H */
