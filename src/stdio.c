#include "stdio.h"
#include "io.h"
#include "memory.h"
#include "errno.h"
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include "syscall.h"

FILE *stdin = NULL;
FILE *stdout = NULL;
FILE *stderr = NULL;

FILE *fopen(const char *path, const char *mode)
{
    int flags = -1;

    if (strcmp(mode, "r") == 0)
        flags = O_RDONLY;
    else if (strcmp(mode, "r+") == 0)
        flags = O_RDWR;
    else if (strcmp(mode, "w") == 0)
        flags = O_WRONLY | O_CREAT | O_TRUNC;
    else if (strcmp(mode, "w+") == 0)
        flags = O_RDWR | O_CREAT | O_TRUNC;
    else if (strcmp(mode, "a") == 0)
        flags = O_WRONLY | O_CREAT | O_APPEND;
    else if (strcmp(mode, "a+") == 0)
        flags = O_RDWR | O_CREAT | O_APPEND;
    else
        return NULL;

    int fd = open(path, flags, 0644);
    if (fd < 0)
        return NULL;

    FILE *f = malloc(sizeof(FILE));
    if (!f) {
        close(fd);
        errno = ENOMEM;
        return NULL;
    }
    f->fd = fd;
    return f;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    if (!stream || size == 0 || nmemb == 0)
        return 0;
    size_t total = size * nmemb;
    ssize_t r = read(stream->fd, ptr, total);
    if (r <= 0)
        return 0;
    return (size_t)r / size;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    if (!stream || size == 0 || nmemb == 0)
        return 0;
    size_t total = size * nmemb;
    ssize_t w = write(stream->fd, ptr, total);
    if (w <= 0)
        return 0;
    return (size_t)w / size;
}

int fclose(FILE *stream)
{
    if (!stream)
        return -1;
    int ret = close(stream->fd);
    free(stream);
    return ret;
}

int fseek(FILE *stream, long offset, int whence)
{
    if (!stream)
        return -1;
    off_t r = lseek(stream->fd, (off_t)offset, whence);
    return r == (off_t)-1 ? -1 : 0;
}

long ftell(FILE *stream)
{
    if (!stream)
        return -1L;
    off_t r = lseek(stream->fd, 0, SEEK_CUR);
    if (r == (off_t)-1)
        return -1L;
    return (long)r;
}

void rewind(FILE *stream)
{
    if (!stream)
        return;
    lseek(stream->fd, 0, SEEK_SET);
}

int fgetc(FILE *stream)
{
    if (!stream)
        return -1;
    unsigned char ch;
    ssize_t r = read(stream->fd, &ch, 1);
    if (r != 1)
        return -1;
    return ch;
}

int fputc(int c, FILE *stream)
{
    if (!stream)
        return -1;
    unsigned char ch = (unsigned char)c;
    ssize_t w = write(stream->fd, &ch, 1);
    if (w != 1)
        return -1;
    return ch;
}

char *fgets(char *s, int size, FILE *stream)
{
    if (!stream || !s || size <= 0)
        return NULL;
    int i = 0;
    while (i < size - 1) {
        unsigned char ch;
        ssize_t r = read(stream->fd, &ch, 1);
        if (r != 1) {
            if (i == 0)
                return NULL;
            break;
        }
        s[i++] = (char)ch;
        if (ch == '\n')
            break;
    }
    s[i] = '\0';
    return s;
}

int fputs(const char *s, FILE *stream)
{
    if (!stream || !s)
        return -1;
    size_t len = strlen(s);
    ssize_t w = write(stream->fd, s, len);
    if (w != (ssize_t)len)
        return -1;
    return (int)w;
}

int fflush(FILE *stream)
{
    if (!stream)
        return 0;
#ifdef SYS_fsync
    long ret = vlibc_syscall(SYS_fsync, stream->fd, 0, 0, 0, 0, 0);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
#endif
    return 0;
}

