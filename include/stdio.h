#ifndef STDIO_H
#define STDIO_H

#include <stddef.h>

typedef struct {
    int fd;
} FILE;

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

FILE *fopen(const char *path, const char *mode);
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
int fclose(FILE *stream);
int fseek(FILE *stream, long offset, int whence);
long ftell(FILE *stream);
void rewind(FILE *stream);

int printf(const char *format, ...);
int fprintf(FILE *stream, const char *format, ...);
int snprintf(char *str, size_t size, const char *format, ...);

/* Basic error helpers */
char *strerror(int errnum);
void perror(const char *s);

#endif /* STDIO_H */
