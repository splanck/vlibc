#ifndef DIRENT_H
#define DIRENT_H

#include <sys/types.h>
#include <stddef.h>

struct dirent {
    ino_t d_ino;
    off_t d_off;
    unsigned short d_reclen;
    unsigned char d_type;
    char d_name[256];
};

typedef struct {
    int fd;
    size_t buf_pos;
    size_t buf_len;
    char buf[512];
    struct dirent ent;
} DIR;

DIR *opendir(const char *name);
struct dirent *readdir(DIR *dirp);
int closedir(DIR *dirp);

#endif /* DIRENT_H */
