#include "stdio.h"
#include "string.h"

struct err_entry {
    int code;
    const char *msg;
};

extern const struct err_entry __vlibc_err_table[];

int strerror_r(int errnum, char *buf, size_t buflen)
{
    if (!buf || buflen == 0)
        return -1;
    for (size_t i = 0; __vlibc_err_table[i].msg; ++i) {
        if (__vlibc_err_table[i].code == errnum) {
            size_t len = strnlen(__vlibc_err_table[i].msg, buflen - 1);
            memcpy(buf, __vlibc_err_table[i].msg, len);
            buf[len] = '\0';
            return 0;
        }
    }
    snprintf(buf, buflen, "Unknown error %d", errnum);
    return 0;
}
