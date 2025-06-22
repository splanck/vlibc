#ifndef DIRENT_H
#define DIRENT_H

#include <sys/types.h>
#include <stddef.h>

/* BSD style directory entry */
struct dirent {
    ino_t          d_fileno;   /* file number */
    off_t          d_off;      /* seek offset to next entry */
    unsigned short d_reclen;   /* length of this record */
    unsigned char  d_type;     /* file type */
    unsigned char  d_namlen;   /* length of name */
    char           d_name[256];/* null terminated file name */
};

#define d_ino d_fileno

typedef struct {
    void *impl;               /* pointer to system DIR structure */
    struct dirent ent;        /* storage for returned entry */
} DIR;

DIR *vlibc_opendir(const char *name);
struct dirent *vlibc_readdir(DIR *dirp);
int vlibc_closedir(DIR *dirp);

#define opendir  vlibc_opendir
#define readdir  vlibc_readdir
#define closedir vlibc_closedir

#endif /* DIRENT_H */
