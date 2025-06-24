#include "stdio.h"
#include "memory.h"
#include "string.h"
#include "errno.h"

FILE *open_memstream(char **bufp, size_t *sizep)
{
    if (!bufp || !sizep) {
        errno = EINVAL;
        return NULL;
    }
    FILE *f = malloc(sizeof(FILE));
    if (!f)
        return NULL;
    memset(f, 0, sizeof(FILE));
    f->fd = -1;
    f->is_mem = 1;
    f->mem_bufp = bufp;
    f->mem_sizep = sizep;
    f->bufsize = 128;
    f->buf = malloc(f->bufsize);
    if (!f->buf) {
        free(f);
        errno = ENOMEM;
        return NULL;
    }
    f->buf_owned = 1;
    *bufp = NULL;
    *sizep = 0;
    return f;
}

FILE *fmemopen(void *buf, size_t size, const char *mode)
{
    if (size == 0) {
        errno = EINVAL;
        return NULL;
    }
    FILE *f = malloc(sizeof(FILE));
    if (!f)
        return NULL;
    memset(f, 0, sizeof(FILE));
    f->fd = -1;
    f->is_mem = 1;
    f->buf = buf ? (unsigned char *)buf : malloc(size);
    if (!f->buf) {
        free(f);
        errno = ENOMEM;
        return NULL;
    }
    f->bufsize = size;
    f->buflen = (mode && mode[0] == 'w' && (!mode[1] || mode[1] != '+')) ? 0 : size;
    f->bufpos = (mode && mode[0] == 'a') ? f->buflen : 0;
    f->buf_owned = buf ? 0 : 1;
    return f;
}
