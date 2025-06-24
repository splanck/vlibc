/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms, with or without modification, are permitted provided that the copyright notice and this permission notice appear in all copies. This software is provided "as is" without warranty.
 *
 * Purpose: Implements the stdio functions for vlibc. Provides wrappers and helpers used by the standard library.
 *
 * Copyright (c) 2025
 */

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

static int flush_buffer(FILE *stream)
{
    if (!stream || !stream->buf || stream->buflen == 0)
        return 0;
    if (stream->is_mem) {
        if (stream->buflen >= stream->bufsize) {
            unsigned char *tmp = realloc(stream->buf, stream->buflen + 1);
            if (!tmp) {
                stream->error = 1;
                return -1;
            }
            stream->buf = tmp;
            stream->bufsize = stream->buflen + 1;
        }
        stream->buf[stream->buflen] = '\0';
        if (stream->mem_bufp)
            *stream->mem_bufp = (char *)stream->buf;
        if (stream->mem_sizep)
            *stream->mem_sizep = stream->buflen;
        return 0;
    }
    size_t off = 0;
    while (off < stream->buflen) {
        ssize_t w = write(stream->fd, stream->buf + off,
                          stream->buflen - off);
        if (w <= 0) {
            stream->error = 1;
            return -1;
        }
        off += (size_t)w;
    }
    stream->buflen = 0;
    stream->bufpos = 0;
    return 0;
}

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
    memset(f, 0, sizeof(FILE));
    f->fd = fd;
    return f;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    if (!stream || size == 0 || nmemb == 0)
        return 0;
    size_t total = size * nmemb;
    size_t copied = 0;
    unsigned char *out = ptr;
    while (copied < total) {
        if (stream->is_mem) {
            if (stream->bufpos >= stream->buflen) {
                stream->eof = 1;
                break;
            }
            size_t avail = stream->buflen - stream->bufpos;
            size_t n = total - copied < avail ? total - copied : avail;
            memcpy(out + copied, stream->buf + stream->bufpos, n);
            stream->bufpos += n;
            copied += n;
            continue;
        }
        if (stream->buf) {
            if (stream->bufpos >= stream->buflen) {
                ssize_t r = read(stream->fd, stream->buf, stream->bufsize);
                if (r <= 0) {
                    if (r == 0)
                        stream->eof = 1;
                    else
                        stream->error = 1;
                    break;
                }
                stream->buflen = (size_t)r;
                stream->bufpos = 0;
            }
            size_t avail = stream->buflen - stream->bufpos;
            size_t n = total - copied < avail ? total - copied : avail;
            memcpy(out + copied, stream->buf + stream->bufpos, n);
            stream->bufpos += n;
            copied += n;
        } else {
            ssize_t r = read(stream->fd, out + copied, total - copied);
            if (r <= 0) {
                if (r == 0)
                    stream->eof = 1;
                else
                    stream->error = 1;
                break;
            }
            copied += (size_t)r;
        }
    }
    return copied / size;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    if (!stream || size == 0 || nmemb == 0)
        return 0;
    size_t total = size * nmemb;
    size_t written = 0;
    const unsigned char *in = ptr;
    while (written < total) {
        if (stream->is_mem) {
            size_t needed = stream->bufpos + (total - written);
            if (needed > stream->bufsize) {
                size_t newsize = stream->bufsize ? stream->bufsize * 2 : 128;
                while (newsize < needed)
                    newsize *= 2;
                unsigned char *tmp = realloc(stream->buf, newsize);
                if (!tmp) {
                    stream->error = 1;
                    break;
                }
                stream->buf = tmp;
                stream->bufsize = newsize;
            }
            size_t n = total - written;
            memcpy(stream->buf + stream->bufpos, in + written, n);
            stream->bufpos += n;
            if (stream->bufpos > stream->buflen)
                stream->buflen = stream->bufpos;
            written += n;
        } else if (stream->buf) {
            if (stream->buflen == stream->bufsize) {
                if (flush_buffer(stream) < 0)
                    break;
            }
            size_t avail = stream->bufsize - stream->buflen;
            size_t n = total - written < avail ? total - written : avail;
            memcpy(stream->buf + stream->buflen, in + written, n);
            stream->buflen += n;
            written += n;
            if (stream->buflen == stream->bufsize) {
                if (flush_buffer(stream) < 0)
                    break;
            }
        } else {
            ssize_t w = write(stream->fd, in + written, total - written);
            if (w <= 0) {
                stream->error = 1;
                break;
            }
            written += (size_t)w;
        }
    }
    return written / size;
}

int fclose(FILE *stream)
{
    if (!stream)
        return -1;
    flush_buffer(stream);
    int ret = 0;
    if (!stream->is_mem)
        ret = close(stream->fd);
    if (stream->is_mem && stream->mem_bufp)
        stream->buf_owned = 0;
    if (stream->buf && stream->buf_owned)
        free(stream->buf);
    free(stream);
    return ret;
}

int fseek(FILE *stream, long offset, int whence)
{
    if (!stream)
        return -1;
    if (flush_buffer(stream) < 0)
        return -1;
    if (stream->is_mem) {
        size_t newpos;
        if (whence == SEEK_SET)
            newpos = (size_t)offset;
        else if (whence == SEEK_CUR)
            newpos = stream->bufpos + (size_t)offset;
        else if (whence == SEEK_END)
            newpos = stream->buflen + (size_t)offset;
        else
            return -1;
        if (newpos > stream->buflen)
            newpos = stream->buflen;
        stream->bufpos = newpos;
        stream->eof = (stream->bufpos >= stream->buflen);
        stream->have_ungot = 0;
        return 0;
    }
    off_t r = lseek(stream->fd, (off_t)offset, whence);
    if (r == (off_t)-1) {
        stream->error = 1;
        return -1;
    }
    stream->bufpos = 0;
    stream->buflen = 0;
    stream->eof = 0;
    stream->have_ungot = 0;
    return 0;
}

long ftell(FILE *stream)
{
    if (!stream)
        return -1L;
    if (flush_buffer(stream) < 0)
        return -1L;
    if (stream->is_mem)
        return (long)stream->bufpos;
    off_t r = lseek(stream->fd, 0, SEEK_CUR);
    if (r == (off_t)-1) {
        stream->error = 1;
        return -1L;
    }
    return (long)r;
}

int fseeko(FILE *stream, off_t offset, int whence)
{
    return fseek(stream, (long)offset, whence);
}

off_t ftello(FILE *stream)
{
    return (off_t)ftell(stream);
}

void rewind(FILE *stream)
{
    if (!stream)
        return;
    flush_buffer(stream);
    if (stream->is_mem) {
        stream->bufpos = 0;
    } else if (lseek(stream->fd, 0, SEEK_SET) == (off_t)-1)
        stream->error = 1;
    stream->bufpos = 0;
    stream->buflen = 0;
    stream->eof = 0;
    stream->error = 0;
    stream->have_ungot = 0;
}

int fgetc(FILE *stream)
{
    if (!stream)
        return -1;
    if (stream->have_ungot) {
        stream->have_ungot = 0;
        return stream->ungot_char;
    }
    unsigned char ch;
    if (fread(&ch, 1, 1, stream) != 1)
        return -1;
    return ch;
}

int fputc(int c, FILE *stream)
{
    if (!stream)
        return -1;
    unsigned char ch = (unsigned char)c;
    if (fwrite(&ch, 1, 1, stream) != 1)
        return -1;
    return ch;
}

int ungetc(int c, FILE *stream)
{
    if (!stream || c == -1 || stream->have_ungot)
        return -1;
    stream->ungot_char = (unsigned char)c;
    stream->have_ungot = 1;
    stream->eof = 0;
    return c & 0xff;
}

char *fgets(char *s, int size, FILE *stream)
{
    if (!stream || !s || size <= 0)
        return NULL;
    int i = 0;
    while (i < size - 1) {
        int c = fgetc(stream);
        if (c == -1) {
            if (i == 0)
                return NULL;
            break;
        }
        s[i++] = (char)c;
        if (c == '\n')
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
    size_t w = fwrite(s, 1, len, stream);
    return (w == len) ? (int)w : -1;
}

int fflush(FILE *stream)
{
    if (!stream)
        return 0;
    if (flush_buffer(stream) < 0)
        return -1;
#if defined(__linux__) && defined(SYS_fsync)
    if (!stream->is_mem) {
        long ret = vlibc_syscall(SYS_fsync, stream->fd, 0, 0, 0, 0, 0);
        if (ret < 0) {
            errno = -ret;
            return -1;
        }
    }
#else
#    ifndef __linux__
    if (!stream->is_mem && fsync(stream->fd) < 0)
        return -1;
#    endif
#endif
    return 0;
}

int setvbuf(FILE *stream, char *buf, int mode, size_t size)
{
    (void)mode; /* buffering mode not currently used */
    if (!stream)
        return -1;
    if (stream->buf && stream->buf_owned)
        free(stream->buf);
    if (size == 0) {
        stream->buf = NULL;
        stream->bufsize = 0;
        stream->bufpos = 0;
        stream->buflen = 0;
        stream->buf_owned = 0;
        return 0;
    }
    if (buf) {
        stream->buf = (unsigned char *)buf;
        stream->buf_owned = 0;
    } else {
        stream->buf = malloc(size);
        if (!stream->buf) {
            errno = ENOMEM;
            return -1;
        }
        stream->buf_owned = 1;
    }
    stream->bufsize = size;
    stream->bufpos = 0;
    stream->buflen = 0;
    return 0;
}

void setbuf(FILE *stream, char *buf)
{
    setvbuf(stream, buf, _IOFBF, BUFSIZ);
}

int feof(FILE *stream)
{
    return stream ? stream->eof : 1;
}

int ferror(FILE *stream)
{
    return stream ? stream->error : 1;
}

void clearerr(FILE *stream)
{
    if (stream) {
        stream->error = 0;
        stream->eof = 0;
        stream->have_ungot = 0;
    }
}

int fileno(FILE *stream)
{
    return stream ? stream->fd : -1;
}

FILE *fdopen(int fd, const char *mode)
{
    (void)mode; /* mode is ignored for now */
    FILE *f = malloc(sizeof(FILE));
    if (!f) {
        errno = ENOMEM;
        return NULL;
    }
    memset(f, 0, sizeof(FILE));
    f->fd = fd;
    return f;
}

