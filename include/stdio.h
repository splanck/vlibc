#ifndef STDIO_H
#define STDIO_H

#include <stddef.h>

typedef struct {
    int fd;
} FILE;

FILE *fopen(const char *path, const char *mode);
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
int fclose(FILE *stream);

#endif /* STDIO_H */
