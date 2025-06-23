#ifndef STDIO_H
#define STDIO_H

#include <stddef.h>
#include <stdarg.h>
#include <sys/types.h>

typedef struct {
    int fd;
    unsigned char *buf;
    size_t bufsize;
    size_t bufpos;
    size_t buflen;
    int buf_owned;
    int error;
    int eof;
    int have_ungot;
    unsigned char ungot_char;
} FILE;

#define _IOFBF 0
#define _IOLBF 1
#define _IONBF 2
#ifndef BUFSIZ
#define BUFSIZ 1024
#endif

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

FILE *fopen(const char *path, const char *mode);
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
int fclose(FILE *stream);
int fseek(FILE *stream, long offset, int whence);
long ftell(FILE *stream);
int fseeko(FILE *stream, off_t offset, int whence);
off_t ftello(FILE *stream);
void rewind(FILE *stream);
int fgetc(FILE *stream);
int fputc(int c, FILE *stream);
int ungetc(int c, FILE *stream);
char *fgets(char *s, int size, FILE *stream);
int fputs(const char *s, FILE *stream);
int fflush(FILE *stream);
int setvbuf(FILE *stream, char *buf, int mode, size_t size);
void setbuf(FILE *stream, char *buf);
int feof(FILE *stream);
int ferror(FILE *stream);
void clearerr(FILE *stream);
int fileno(FILE *stream);
FILE *fdopen(int fd, const char *mode);

int printf(const char *format, ...);
int fprintf(FILE *stream, const char *format, ...);
int snprintf(char *str, size_t size, const char *format, ...);
int sprintf(char *str, const char *format, ...);
int asprintf(char **strp, const char *format, ...);
int vasprintf(char **strp, const char *format, va_list ap);
int vprintf(const char *format, va_list ap);
int vfprintf(FILE *stream, const char *format, va_list ap);
int vsnprintf(char *str, size_t size, const char *format, va_list ap);
int vsprintf(char *str, const char *format, va_list ap);
/*
 * The scanf family supports %d, %u, %x, %o, %s, and floating point
 * formats like %f, %lf, and %g.
 */
int vscanf(const char *format, va_list ap);
int vfscanf(FILE *stream, const char *format, va_list ap);
int vsscanf(const char *str, const char *format, va_list ap);
int scanf(const char *format, ...);
int fscanf(FILE *stream, const char *format, ...);
int sscanf(const char *str, const char *format, ...);

/* Basic error helpers */
char *strerror(int errnum);
int strerror_r(int errnum, char *buf, size_t buflen);
void perror(const char *s);

FILE *popen(const char *command, const char *mode);
int pclose(FILE *stream);

int mkstemp(char *template);
FILE *tmpfile(void);
char *tmpnam(char *s);
char *tempnam(const char *dir, const char *pfx);

ssize_t getdelim(char **lineptr, size_t *n, int delim, FILE *stream);
ssize_t getline(char **lineptr, size_t *n, FILE *stream);

#endif /* STDIO_H */
