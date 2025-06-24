#include "unistd.h"
#include "fcntl.h"
#include "termios.h"
#include "string.h"
#include "io.h"

char *getpass(const char *prompt)
{
    static char buf[128];
    int fd = open("/dev/tty", O_RDWR);
    if (fd < 0)
        fd = STDIN_FILENO;

    if (prompt)
        write(fd, prompt, strlen(prompt));

    struct termios oldt;
    struct termios newt;
    int have_termios = (tcgetattr(fd, &oldt) == 0);
    if (have_termios) {
        newt = oldt;
        newt.c_lflag &= ~(unsigned)ECHO;
        tcsetattr(fd, TCSANOW, &newt);
    }

    size_t i = 0;
    while (i < sizeof(buf) - 1) {
        char c;
        ssize_t n = read(fd, &c, 1);
        if (n <= 0 || c == '\n' || c == '\r')
            break;
        buf[i++] = c;
    }
    buf[i] = '\0';

    if (have_termios)
        tcsetattr(fd, TCSANOW, &oldt);
    write(fd, "\n", 1);

    if (fd != STDIN_FILENO)
        close(fd);
    return buf;
}
