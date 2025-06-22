#include "netdb.h"
#include <sys/utsname.h>
#include <string.h>

extern int host_uname(struct utsname *buf) __asm__("uname");

int gethostname(char *name, size_t len)
{
    struct utsname uts;
    if (host_uname(&uts) != 0)
        return -1;
    if (len > 0) {
        strncpy(name, uts.nodename, len);
        name[len - 1] = '\0';
    }
    return 0;
}
