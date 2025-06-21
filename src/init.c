#include "vlibc.h"
#include "stdio.h"
#include "memory.h"

void vlibc_init(void)
{
    stdin = malloc(sizeof(FILE));
    if (stdin)
        stdin->fd = 0;

    stdout = malloc(sizeof(FILE));
    if (stdout)
        stdout->fd = 1;

    stderr = malloc(sizeof(FILE));
    if (stderr)
        stderr->fd = 2;
}
