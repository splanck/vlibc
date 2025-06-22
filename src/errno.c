#include "errno.h"

/* each thread maintains its own errno value */
__thread int errno;

int *__errno_location(void)
{
    return &errno;
}
