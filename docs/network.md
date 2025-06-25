## Networking

The socket layer exposes thin wrappers around the kernel's networking
syscalls including `socket`, `bind`, `listen`, `accept`, `connect`,
`getsockname`, `getpeername`, `shutdown`, `socketpair`, `send`, `recv`,
`sendto`, `recvfrom`, `setsockopt`, and `getsockopt`.
Address resolution is handled
via `getaddrinfo`, `freeaddrinfo`, and `getnameinfo`.

Utilities `inet_pton` and `inet_ntop` convert between IPv4 or IPv6
presentation strings and binary network format. Legacy helpers
`inet_aton` and `inet_ntoa` perform the same conversion for IPv4
addresses only. Use `htons`, `ntohs`, `htonl`, and `ntohl` to convert
between host and network byte order.

```c
struct in_addr ip;
if (inet_aton("127.0.0.1", &ip))
    printf("%s\n", inet_ntoa(ip));
```

```c
struct addrinfo *ai;
if (getaddrinfo("localhost", "80", NULL, &ai) == 0) {
int fd = socket(AF_INET, SOCK_STREAM, 0);
    connect(fd, ai->ai_addr, ai->ai_addrlen);
    freeaddrinfo(ai);
}
```

When `getaddrinfo` fails it returns an `EAI_*` code. Use
`gai_strerror` to obtain a human readable description:

```c
struct addrinfo *ai;
int r = getaddrinfo("nosuch.host", NULL, NULL, &ai);
if (r != 0)
    fprintf(stderr, "%s\n", gai_strerror(r));
```

### Legacy Lookup API

Older programs may still rely on `gethostbyname` and `gethostbyaddr`.
vlibc implements these wrappers by consulting `/etc/hosts` and falling
back to `getaddrinfo` when no entry is found.

```c
struct hostent {
    char *h_name;
    char **h_aliases;
    int h_addrtype;
    int h_length;
    char **h_addr_list;
};

struct hostent *gethostbyname(const char *name);
struct hostent *gethostbyaddr(const void *addr, socklen_t len, int type);
```

Create a pair of connected sockets with `socketpair`:

```c
int sv[2];
if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == 0) {
    send(sv[0], "hi", 2, 0);
    char buf[3] = {0};
    recv(sv[1], buf, 2, 0);
}
```

Network interfaces can be enumerated with `getifaddrs` which fills a list of
`struct ifaddrs`. Each entry describes an interface name, flags and optional
addresses. Free the list with `freeifaddrs` when done.

```c
struct ifaddrs *ifas;
if (getifaddrs(&ifas) == 0) {
    for (struct ifaddrs *i = ifas; i; i = i->ifa_next)
        printf("%s\n", i->ifa_name);
    freeifaddrs(ifas);
}
```

## I/O Multiplexing

`select` and `poll` wait for activity on multiple file descriptors.

```c
fd_set set;
FD_ZERO(&set);
FD_SET(fd, &set);
select(fd + 1, &set, NULL, NULL, NULL);
```

On BSD systems these wrappers simply call the host C library's
implementations which may exhibit slightly different semantics than the
Linux syscall-based versions.

