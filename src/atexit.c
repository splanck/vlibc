#include "stdlib.h"

#define ATEXIT_MAX 32

static void (*handlers[ATEXIT_MAX])(void);
static int handler_count = 0;

int atexit(void (*fn)(void))
{
    if (!fn || handler_count >= ATEXIT_MAX)
        return -1;
    handlers[handler_count++] = fn;
    return 0;
}

void __run_atexit(void)
{
    for (int i = handler_count - 1; i >= 0; --i) {
        if (handlers[i])
            handlers[i]();
    }
}
