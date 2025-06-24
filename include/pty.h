#ifndef PTY_H
#define PTY_H

#include <sys/types.h>
#include <termios.h>
#include "sys/ioctl.h"

int openpty(int *amaster, int *aslave, char *name,
            struct termios *termp, struct winsize *winp);
int forkpty(int *amaster, char *name,
            struct termios *termp, struct winsize *winp);

#endif /* PTY_H */
