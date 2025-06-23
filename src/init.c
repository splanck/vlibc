#include "vlibc.h"
#include "stdio.h"
#include "memory.h"
#include "string.h"

void vlibc_init(void)
{
    stdin = malloc(sizeof(FILE));
    if (stdin) {
        memset(stdin, 0, sizeof(FILE));
        stdin->fd = 0;
    }

    stdout = malloc(sizeof(FILE));
    if (stdout) {
        memset(stdout, 0, sizeof(FILE));
        stdout->fd = 1;
    }

    stderr = malloc(sizeof(FILE));
    if (stderr) {
        memset(stderr, 0, sizeof(FILE));
        stderr->fd = 2;
    }
}
