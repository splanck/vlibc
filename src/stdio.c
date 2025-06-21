#include "stdio.h"
#include "io.h"
#include "memory.h"
#include "errno.h"
#include <fcntl.h>
#include <string.h>

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

