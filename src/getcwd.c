#include "stdlib.h"

extern char *host_getcwd(char *buf, size_t size) __asm__("getcwd");

char *getcwd(char *buf, size_t size)
{
    return host_getcwd(buf, size);
}
