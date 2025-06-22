#include "stdlib.h"
#include "process.h"

void abort(void)
{
    kill(getpid(), SIGABRT);
    _exit(1);
}
