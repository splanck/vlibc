#ifndef UNISTD_H
#define UNISTD_H

#if defined(__has_include)
#  if __has_include_next(<unistd.h>)
#    include_next <unistd.h>
#  endif
#endif

int isatty(int fd);

#endif /* UNISTD_H */
