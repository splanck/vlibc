#include "stdio.h"
#include "memory.h"
#include "errno.h"

ssize_t getdelim(char **lineptr, size_t *n, int delim, FILE *stream)
{
    if (!lineptr || !n || !stream) {
        errno = EINVAL;
        return -1;
    }
    if (!*lineptr || *n == 0) {
        *n = *n ? *n : 128;
        *lineptr = malloc(*n);
        if (!*lineptr) {
            errno = ENOMEM;
            return -1;
        }
    }
    size_t pos = 0;
    int c;
    while ((c = fgetc(stream)) != -1) {
        if (pos + 1 >= *n) {
            size_t new_size = *n * 2;
            char *tmp = realloc(*lineptr, new_size);
            if (!tmp) {
                errno = ENOMEM;
                return -1;
            }
            *lineptr = tmp;
            *n = new_size;
        }
        (*lineptr)[pos++] = (char)c;
        if (c == delim)
            break;
    }
    if (c == -1 && pos == 0)
        return -1;
    (*lineptr)[pos] = '\0';
    return (ssize_t)pos;
}

ssize_t getline(char **lineptr, size_t *n, FILE *stream)
{
    return getdelim(lineptr, n, '\n', stream);
}
