#include "stdlib.h"

static unsigned long rand_state = 1;

int rand(void)
{
    rand_state = rand_state * 1103515245 + 12345;
    return (int)((rand_state >> 16) & 0x7fff);
}

void srand(unsigned seed)
{
    rand_state = seed;
}

