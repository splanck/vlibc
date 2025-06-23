#include "termios.h"

extern int host_tcgetattr(int, struct termios *) __asm__("tcgetattr");
extern int host_tcsetattr(int, int, const struct termios *) __asm__("tcsetattr");
extern void host_cfmakeraw(struct termios *) __asm__("cfmakeraw");

int tcgetattr(int fd, struct termios *t)
{
    return host_tcgetattr(fd, t);
}

int tcsetattr(int fd, int act, const struct termios *t)
{
    return host_tcsetattr(fd, act, t);
}

void cfmakeraw(struct termios *t)
{
    host_cfmakeraw(t);
}
