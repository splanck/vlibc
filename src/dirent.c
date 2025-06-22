#define _GNU_SOURCE
#include <dirent.h>
#include "dirent.h"
#undef opendir
#undef readdir
#undef closedir

DIR *vlibc_opendir(const char *name)
{
    return opendir(name);
}

struct dirent *vlibc_readdir(DIR *dirp)
{
    return readdir(dirp);
}

int vlibc_closedir(DIR *dirp)
{
    return closedir(dirp);
}
