#include "stdlib.h"
#include "string.h"
#include "memory.h"
#include "errno.h"

char *realpath(const char *path, char *resolved_path)
{
    if (!path) {
        errno = EINVAL;
        return NULL;
    }

    char cwd[256];
    char *full = NULL;
    if (path[0] != '/') {
        if (!getcwd(cwd, sizeof(cwd)))
            return NULL;
        size_t cwd_len = strlen(cwd);
        size_t add_slash = (cwd_len > 1 && cwd[cwd_len-1] != '/') ? 1 : 0;
        size_t len = cwd_len + add_slash + strlen(path) + 1;
        full = malloc(len + 1);
        if (!full)
            return NULL;
        strcpy(full, cwd);
        if (add_slash)
            strcat(full, "/");
        strcat(full, path);
    } else {
        full = strdup(path);
        if (!full)
            return NULL;
    }

    size_t out_cap = strlen(full) + 2;
    char *outbuf = resolved_path ? resolved_path : malloc(out_cap);
    if (!outbuf) {
        free(full);
        return NULL;
    }

    size_t out_len = 1;
    outbuf[0] = '/';
    outbuf[1] = '\0';

    char *p = full;
    if (*p == '/')
        ++p;
    while (*p) {
        while (*p == '/')
            ++p;
        if (!*p)
            break;
        char *start = p;
        while (*p && *p != '/')
            ++p;
        size_t seg_len = p - start;
        if (seg_len == 1 && start[0] == '.') {
            /* ignore */
        } else if (seg_len == 2 && start[0] == '.' && start[1] == '.') {
            if (out_len > 1) {
                while (out_len > 1 && outbuf[out_len-1] != '/')
                    --out_len;
                if (out_len > 1)
                    --out_len; /* drop trailing '/' */
            }
        } else {
            if (out_len > 1)
                outbuf[out_len++] = '/';
            memcpy(outbuf + out_len, start, seg_len);
            out_len += seg_len;
            outbuf[out_len] = '\0';
        }
    }

    if (out_len > 1 && outbuf[out_len-1] == '/') {
        outbuf[out_len-1] = '\0';
    }

    free(full);
    return outbuf;
}

