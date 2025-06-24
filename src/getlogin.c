#include "unistd.h"
#include "pwd.h"
#include "string.h"

char *getlogin(void)
{
    static __thread char name[64];
    if (name[0])
        return name;
    struct passwd *pw = getpwuid(getuid());
    if (!pw || !pw->pw_name)
        return NULL;
    strncpy(name, pw->pw_name, sizeof(name) - 1);
    name[sizeof(name) - 1] = '\0';
    return name;
}
