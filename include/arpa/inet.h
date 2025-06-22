#ifndef ARPA_INET_H
#define ARPA_INET_H

#include <netinet/in.h>

int inet_pton(int af, const char *src, void *dst);
const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);

#endif /* ARPA_INET_H */
