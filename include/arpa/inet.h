#ifndef ARPA_INET_H
#define ARPA_INET_H

#include <netinet/in.h>

#ifndef INET_ADDRSTRLEN
#define INET_ADDRSTRLEN 16
#endif

#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN 46
#endif

int inet_pton(int af, const char *src, void *dst);
const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);
int inet_aton(const char *cp, struct in_addr *inp);
char *inet_ntoa(struct in_addr in);

#endif /* ARPA_INET_H */
