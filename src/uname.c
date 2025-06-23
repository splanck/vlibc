#include "sys/utsname.h"
#include "errno.h"

#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
#include <sys/sysctl.h>
#include "string.h"

int uname(struct utsname *u)
{
    size_t n;
    n = sizeof(u->sysname);
    if (sysctlbyname("kern.ostype", u->sysname, &n, NULL, 0) < 0)
        return -1;
    if (n >= sizeof(u->sysname))
        u->sysname[sizeof(u->sysname) - 1] = '\0';

    n = sizeof(u->nodename);
    if (sysctlbyname("kern.hostname", u->nodename, &n, NULL, 0) < 0)
        return -1;
    if (n >= sizeof(u->nodename))
        u->nodename[sizeof(u->nodename) - 1] = '\0';

    n = sizeof(u->release);
    if (sysctlbyname("kern.osrelease", u->release, &n, NULL, 0) < 0)
        return -1;
    if (n >= sizeof(u->release))
        u->release[sizeof(u->release) - 1] = '\0';

    n = sizeof(u->version);
    if (sysctlbyname("kern.version", u->version, &n, NULL, 0) < 0)
        return -1;
    if (n >= sizeof(u->version))
        u->version[sizeof(u->version) - 1] = '\0';

    n = sizeof(u->machine);
    if (sysctlbyname("hw.machine", u->machine, &n, NULL, 0) < 0)
        return -1;
    if (n >= sizeof(u->machine))
        u->machine[sizeof(u->machine) - 1] = '\0';

    return 0;
}

#else
extern int host_uname(struct utsname *) __asm__("uname");

int uname(struct utsname *u)
{
    return host_uname(u);
}
#endif

