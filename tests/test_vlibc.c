/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms, with or without modification, are permitted provided that the copyright notice and this permission notice appear in all copies. This software is provided "as is" without warranty.
 *
 * Purpose: Contains unit tests for vlibc using a minimal test framework. Exercises various library features.
 *
 * Copyright (c) 2025
 */

#include "minunit.h"
#include "../include/memory.h"
#include "../include/io.h"
#include "../include/sys/socket.h"
#include "../include/sys/uio.h"
#include "../include/sys/file.h"
#include <netinet/in.h>
#include "../include/arpa/inet.h"
#include <stdint.h>
#include <inttypes.h>
#include <stddef.h>
#include "../include/sys/stat.h"
#include "../include/stdio.h"
#include "../include/pthread.h"
#include "../include/util.h"
#include "../include/semaphore.h"
#include "../include/sys/select.h"
#include "../include/poll.h"
#include <dirent.h>
#include "../include/vlibc.h"
#include "../include/dlfcn.h"

#include <fcntl.h>
#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif
#include "../include/string.h"
#include "../include/stdlib.h"
#include "../include/wchar.h"
#include "../include/wctype.h"
#include "../include/iconv.h"
#include "../include/env.h"
#include "../include/sys/utsname.h"
#include "../include/pwd.h"
#include "../include/grp.h"
#include "../include/process.h"
#include "../include/getopt.h"
#include "../include/math.h"
#include "../include/fenv.h"
#include "../include/complex.h"
#include "../include/locale.h"
#include "../include/regex.h"
#include "../include/ftw.h"
#include "../include/fts.h"
#include "../include/wordexp.h"
#include "../include/vis.h"
#include "../include/search.h"
#include "../include/pty.h"
#include "../include/termios.h"
#ifndef B9600
#define B9600 13
#endif
#ifndef B38400
#define B38400 15
#endif
#include <unistd.h>
#include <stdio.h>
#include "../include/err.h"
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include "../include/setjmp.h"
#include "../include/time.h"
#include "../include/sys/resource.h"
#include "../include/sys/times.h"
#include "../include/sys/mman.h"
#include "../include/sys/shm.h"
#include "../include/sys/sem.h"
#include "../include/sys/msg.h"
#include "../include/mqueue.h"
#include "../include/sched.h"

/* use host printf for test output */
int printf(const char *fmt, ...);

int tests_run = 0;

typedef const char *(*test_func)(void);
struct test_case {
    const char *name;
    const char *category;
    test_func func;
};

static void handle_usr1(int);
static void *send_signal(void *);

#define REGISTER_TEST(cat, fn) { #fn, cat, fn }
/* convenience wrappers for test categories */
#define MEM_TEST(fn)       REGISTER_TEST("memory", fn)
#define IO_TEST(fn)        REGISTER_TEST("io", fn)
#define NET_TEST(fn)       REGISTER_TEST("network", fn)
#define THREAD_TEST(fn)    REGISTER_TEST("thread", fn)
#define PROC_TEST(fn)      REGISTER_TEST("process", fn)
#define TIME_TEST(fn)      REGISTER_TEST("time", fn)
#define FS_TEST(fn)        REGISTER_TEST("fs", fn)
#define ENV_TEST(fn)       REGISTER_TEST("env", fn)
#define UTIL_TEST(fn)      REGISTER_TEST("util", fn)
#define MATH_TEST(fn)      REGISTER_TEST("math", fn)
#define REGEX_TEST(fn)     REGISTER_TEST("regex", fn)
#define CRYPTO_TEST(fn)    REGISTER_TEST("crypto", fn)

static int exit_pipe[2];

static void atexit_handler(void)
{
    ssize_t r = write(exit_pipe[1], "x", 1);
    if (r < 0)
        perror("write");
}

static const char *test_byte_order(void)
{
    uint16_t v16 = 0x1234;
    uint32_t v32 = 0x12345678;
    uint16_t n16 = htons(v16);
    uint32_t n32 = htonl(v32);
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    mu_assert("htons", n16 == 0x3412);
    mu_assert("htonl", n32 == 0x78563412);
#else
    mu_assert("htons", n16 == v16);
    mu_assert("htonl", n32 == v32);
#endif
    mu_assert("ntohs", ntohs(n16) == v16);
    mu_assert("ntohl", ntohl(n32) == v32);
    return 0;
}



static void *thread_fn(void *arg)
{
    int *p = arg;
    *p = 42;
    return (void *)123;
}

static void *strerror_r_worker(void *arg)
{
    int err = *(int *)arg;
    char buf[64];
    if (strerror_r(err, buf, sizeof(buf)) != 0)
        return (void *)1;
    if (err == ENOENT)
        return (void *)(strcmp(buf, "No such file or directory") != 0);
    char expect[32];
    snprintf(expect, sizeof(expect), "Unknown error %d", err);
    return (void *)(strcmp(buf, expect) != 0);
}

struct asctime_arg {
    struct tm tm;
    const char *expect;
};

static void *asctime_r_worker(void *arg)
{
    struct asctime_arg *a = arg;
    char buf[32];
    if (!asctime_r(&a->tm, buf))
        return (void *)1;
    return (void *)(strcmp(buf, a->expect) != 0);
}

static const char *test_malloc(void)
{
    void *p = malloc(16);
    mu_assert("malloc returned NULL", p != NULL);
    vmemset(p, 0xAA, 16);
    free(p);
    return 0;
}

static const char *test_malloc_reuse(void)
{
    void *a = malloc(32);
    void *b = malloc(64);
    void *c = malloc(16);

    mu_assert("alloc a", a != NULL);
    mu_assert("alloc b", b != NULL);
    mu_assert("alloc c", c != NULL);

    free(b);
    free(a);

    void *d = malloc(24);
    void *e = malloc(8);

    mu_assert("reuse d", d == a);
    mu_assert("reuse e", e == b);

    free(c);
    free(d);
    free(e);
    return 0;
}

static const char *test_reallocf_fail(void)
{
    void *p = malloc(32);
    mu_assert("alloc p", p != NULL);

    void *r = reallocf(p, SIZE_MAX / 2);
    mu_assert("reallocf NULL", r == NULL);

    void *q = malloc(16);
    mu_assert("reuse after reallocf", q == p);
    free(q);
    return 0;
}

static const char *test_posix_memalign_basic(void)
{
    void *p = NULL;
    int r = posix_memalign(&p, 64, 32);
    mu_assert("posix_memalign ret", r == 0);
    mu_assert("ptr aligned", ((uintptr_t)p & 63) == 0);
    vmemset(p, 0xBB, 32);
    free(p);
    return 0;
}

static const char *test_posix_memalign(void)
{
    void *p = NULL;
    int r = posix_memalign(&p, 64, 64);
    mu_assert("posix_memalign ret", r == 0);
    mu_assert("ptr aligned", ((uintptr_t)p % 64) == 0);
    char *c = p;
    for (int i = 0; i < 64; i++)
        c[i] = (char)i;
    free(p);
    return 0;
}

static const char *test_aligned_alloc(void)
{
    void *p = aligned_alloc(32, 64);
    mu_assert("aligned_alloc ptr", p != NULL);
    mu_assert("ptr aligned", ((uintptr_t)p % 32) == 0);
    char *c = p;
    for (int i = 0; i < 64; i++)
        c[i] = (char)i;
    free(p);
    return 0;
}

static const char *test_posix_memalign_overflow(void)
{
    void *p = (void *)1;
    int r = posix_memalign(&p, 16, SIZE_MAX);
    mu_assert("overflow ENOMEM", r == ENOMEM);
    mu_assert("memptr unchanged", p == (void *)1);
    return 0;
}

static const char *test_reallocarray_overflow(void)
{
    size_t big = (size_t)-1 / 2 + 1;
    errno = 0;
    void *p = reallocarray(NULL, big, 2);
    mu_assert("overflow NULL", p == NULL);
    mu_assert("errno ENOMEM", errno == ENOMEM);
    return 0;
}

static const char *test_reallocarray_basic(void)
{
    char *p = reallocarray(NULL, 4, 8);
    mu_assert("alloc", p != NULL);
    p[0] = 'z';
    p = reallocarray(p, 8, 8);
    mu_assert("realloc", p != NULL);
    mu_assert("preserve", p[0] == 'z');
    free(p);
    return 0;
}

static const char *test_recallocarray_grow(void)
{
    int *p = recallocarray(NULL, 2, sizeof(int));
    mu_assert("alloc", p != NULL);
    p[0] = 1;
    p[1] = 2;
    p = recallocarray(p, 4, sizeof(int));
    mu_assert("recalloc", p != NULL);
    mu_assert("preserve0", p[0] == 1);
    mu_assert("preserve1", p[1] == 2);
    mu_assert("zero", p[2] == 0 && p[3] == 0);
    free(p);
    return 0;
}

static const char *test_memory_ops(void)
{
    char buf[8];
    vmemset(buf, 'x', sizeof(buf));
    for (size_t i = 0; i < sizeof(buf); i++)
        mu_assert("vmemset failed", buf[i] == 'x');

    char src[8] = "abcdefg";
    vmemcpy(buf, src, 8);
    mu_assert("vmemcpy failed", vmemcmp(buf, src, 8) == 0);

    vmemmove(buf + 1, buf, 7);
    mu_assert("vmemmove failed", buf[1] == 'a' && buf[2] == 'b');

    mu_assert("vmemcmp diff", vmemcmp("abc", "abd", 3) < 0);

    memset(buf, 'y', sizeof(buf));
    for (size_t i = 0; i < sizeof(buf); i++)
        mu_assert("memset failed", buf[i] == 'y');

    memcpy(buf, src, 8);
    mu_assert("memcpy failed", memcmp(buf, src, 8) == 0);

    memmove(buf + 2, buf, 6);
    mu_assert("memmove std failed", buf[2] == 'a' && buf[3] == 'b');

    mu_assert("memcmp diff std", memcmp("abc", "abd", 3) < 0);
    return 0;
}

static const char *test_io(void)
{
    const char *fname = "tmp_test_file";
    int fd = open(fname, O_CREAT | O_RDWR, 0644);
    mu_assert("open failed", fd >= 0);

    const char *msg = "abc";
    ssize_t w = write(fd, msg, strlen(msg));
    mu_assert("write failed", w == (ssize_t)strlen(msg));

    lseek(fd, 0, SEEK_SET);
    char buf[4] = {0};
    ssize_t r = read(fd, buf, 3);
    mu_assert("read failed", r == 3);
    mu_assert("content mismatch", strncmp(msg, buf, 3) == 0);

    close(fd);
    unlink(fname);
    return 0;
}

static const char *test_lseek_dup(void)
{
    const char *fname = "tmp_dup_file";
    int fd = open(fname, O_CREAT | O_RDWR, 0644);
    mu_assert("open failed", fd >= 0);

    const char *msg = "hello";
    ssize_t w = write(fd, msg, strlen(msg));
    mu_assert("write failed", w == (ssize_t)strlen(msg));

    off_t off = lseek(fd, 0, SEEK_CUR);
    mu_assert("lseek cur", off == (off_t)strlen(msg));

    off = lseek(fd, 0, SEEK_SET);
    mu_assert("lseek set", off == 0);

    int fd2 = dup(fd);
    mu_assert("dup failed", fd2 >= 0);

    char buf[8] = {0};
    ssize_t r = read(fd2, buf, sizeof(buf) - 1);
    mu_assert("dup read", r == (ssize_t)strlen(msg));
    mu_assert("dup content", strcmp(buf, msg) == 0);

    const char *msg2 = "world";
    lseek(fd, 0, SEEK_SET);
    w = write(fd2, msg2, strlen(msg2));
    mu_assert("write via dup", w == (ssize_t)strlen(msg2));

    lseek(fd, 0, SEEK_SET);
    char buf2[16] = {0};
    r = read(fd, buf2, sizeof(buf2) - 1);
    mu_assert("read after dup", r == (ssize_t)strlen(msg2));
    mu_assert("content after dup", strncmp(buf2, msg2, strlen(msg2)) == 0);

    int fd3 = dup2(fd2, fd);
    mu_assert("dup2 failed", fd3 == fd);

    lseek(fd3, 0, SEEK_SET);
    const char *msg3 = "abc";
    w = write(fd3, msg3, strlen(msg3));
    mu_assert("write via dup2", w == (ssize_t)strlen(msg3));

    lseek(fd2, 0, SEEK_SET);
    char buf3[4] = {0};
    r = read(fd2, buf3, 3);
    mu_assert("read after dup2", r == 3);
    mu_assert("content after dup2", strncmp(buf3, msg3, 3) == 0);

    close(fd2);
    close(fd3);
    unlink(fname);
    return 0;
}

static const char *test_lseek_negative_offset(void)
{
    const char *fname = "tmp_neg_seek";
    int fd = open(fname, O_CREAT | O_RDWR, 0644);
    mu_assert("open failed", fd >= 0);

    const char *msg = "abcdef";
    mu_assert("write", write(fd, msg, strlen(msg)) == (ssize_t)strlen(msg));

    off_t off = lseek(fd, -2, SEEK_END);
    mu_assert("seek result", off == (off_t)strlen(msg) - 2);

    close(fd);
    unlink(fname);
    return 0;
}

static const char *test_pread_pwrite(void)
{
    const char *fname = "tmp_pread_file";
    int fd = open(fname, O_CREAT | O_RDWR, 0644);
    mu_assert("open", fd >= 0);

    const char *msg = "abcdef";
    mu_assert("init write", write(fd, msg, 6) == 6);
    off_t pos = lseek(fd, 0, SEEK_CUR);
    mu_assert("pos", pos == 6);

    const char *patch = "XY";
    ssize_t w = pwrite(fd, patch, 2, 2);
    mu_assert("pwrite", w == 2);
    mu_assert("offset unchanged", lseek(fd, 0, SEEK_CUR) == pos);

    char buf[5] = {0};
    ssize_t r = pread(fd, buf, 4, 1);
    mu_assert("pread", r == 4);
    mu_assert("pread data", strncmp(buf, "bXYe", 4) == 0);
    mu_assert("offset still", lseek(fd, 0, SEEK_CUR) == pos);

    close(fd);
    unlink(fname);
    return 0;
}

static const char *test_readv_writev(void)
{
    const char *fname = "tmp_vec_file";
    int fd = open(fname, O_CREAT | O_RDWR, 0644);
    mu_assert("open", fd >= 0);

    struct iovec wv[2];
    const char *a = "ab";
    const char *b = "cd";
    wv[0].iov_base = (void *)a;
    wv[0].iov_len = 2;
    wv[1].iov_base = (void *)b;
    wv[1].iov_len = 2;
    mu_assert("writev", writev(fd, wv, 2) == 4);

    lseek(fd, 0, SEEK_SET);
    char buf1[3] = {0};
    char buf2[3] = {0};
    struct iovec rv[2];
    rv[0].iov_base = buf1;
    rv[0].iov_len = 2;
    rv[1].iov_base = buf2;
    rv[1].iov_len = 2;
    mu_assert("readv", readv(fd, rv, 2) == 4);
    mu_assert("vec data", strcmp(buf1, "ab") == 0 && strcmp(buf2, "cd") == 0);

    close(fd);
    unlink(fname);
    return 0;
}

static const char *test_preadv_pwritev(void)
{
    const char *fname = "tmp_preadv_file";
    int fd = open(fname, O_CREAT | O_RDWR, 0644);
    mu_assert("open", fd >= 0);

    const char *msg = "abcdef";
    mu_assert("init write", write(fd, msg, 6) == 6);
    off_t pos = lseek(fd, 0, SEEK_CUR);
    mu_assert("pos", pos == 6);

    const char *a = "XY";
    const char *b = "ZZ";
    struct iovec wv[2];
    wv[0].iov_base = (void *)a;
    wv[0].iov_len = 2;
    wv[1].iov_base = (void *)b;
    wv[1].iov_len = 2;
    ssize_t w = pwritev(fd, wv, 2, 2);
    mu_assert("pwritev", w == 4);
    mu_assert("offset unchanged", lseek(fd, 0, SEEK_CUR) == pos);

    char buf1[3] = {0};
    char buf2[3] = {0};
    struct iovec rv[2];
    rv[0].iov_base = buf1;
    rv[0].iov_len = 2;
    rv[1].iov_base = buf2;
    rv[1].iov_len = 2;
    ssize_t r = preadv(fd, rv, 2, 2);
    mu_assert("preadv", r == 4);
    mu_assert("vec data", strcmp(buf1, "XY") == 0 && strcmp(buf2, "ZZ") == 0);
    mu_assert("offset still", lseek(fd, 0, SEEK_CUR) == pos);

    close(fd);
    unlink(fname);
    return 0;
}

static const char *test_sendfile_copy(void)
{
    const char *src = "tmp_sf_src";
    const char *dst = "tmp_sf_dst";
    int in = open(src, O_CREAT | O_RDWR | O_TRUNC, 0644);
    mu_assert("open src", in >= 0);
    const char *msg = "sendfile";
    mu_assert("write src", write(in, msg, strlen(msg)) == (ssize_t)strlen(msg));
    close(in);

    in = open(src, O_RDONLY);
    int out;
    off_t sent;
    int r;

    out = open(dst, O_CREAT | O_RDWR | O_TRUNC, 0644);
    mu_assert("open out", out >= 0 && in >= 0);
    sent = 0;
    r = sendfile(in, out, 0, strlen(msg), NULL, &sent, 0);
    mu_assert("sendfile", r == 0 && sent == (off_t)strlen(msg));
    close(out);

    out = open(dst, O_CREAT | O_RDWR | O_TRUNC, 0644);
    mu_assert("open out2", out >= 0);
    sent = 0;
    r = sendfile(in, out, 0, strlen(msg) + 4, NULL, &sent, 0);
    mu_assert("sendfile partial", r == 0 && sent == (off_t)strlen(msg));
    close(out);

    out = open(dst, O_CREAT | O_RDWR | O_TRUNC, 0644);
    mu_assert("open out3", out >= 0);
    r = sendfile(in, out, 0, strlen(msg), NULL, NULL, 0);
    mu_assert("sendfile result", r == (int)strlen(msg));

    lseek(out, 0, SEEK_SET);
    char buf[16] = {0};
    mu_assert("read dst", read(out, buf, sizeof(buf)) == (ssize_t)strlen(msg));
    mu_assert("content", strcmp(buf, msg) == 0);

    close(in);
    close(out);
    unlink(src);
    unlink(dst);
    return 0;
}

#ifdef __NetBSD__
static const char *test_sendfile_socket(void)
{
    const char *src = "tmp_sf_sock_src";
    int in = open(src, O_CREAT | O_RDWR | O_TRUNC, 0644);
    mu_assert("open src", in >= 0);
    const char *msg = "netbsd";
    mu_assert("write src", write(in, msg, strlen(msg)) == (ssize_t)strlen(msg));
    lseek(in, 0, SEEK_SET);

    int sv[2];
    mu_assert("socketpair", socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == 0);

    off_t sent = 0;
    int r = sendfile(in, sv[0], 0, strlen(msg), NULL, &sent, 0);
    mu_assert("sendfile", r == 0 && sent == (off_t)strlen(msg));

    char buf[16] = {0};
    mu_assert("recv", read(sv[1], buf, sizeof(buf)) == (ssize_t)strlen(msg));
    mu_assert("content", strcmp(buf, msg) == 0);

    close(in);
    close(sv[0]);
    close(sv[1]);
    unlink(src);
    return 0;
}
#endif

static const char *test_socket(void)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    mu_assert("socket creation failed", fd >= 0);
    if (fd >= 0)
        close(fd);
    return 0;
}

static const char *test_socketpair_basic(void)
{
    int sv[2];
    mu_assert("socketpair", socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == 0);
    const char *msg = "ok";
    ssize_t w = write(sv[0], msg, 2);
    mu_assert("write", w == 2);
    char buf[3] = {0};
    ssize_t r = read(sv[1], buf, 2);
    mu_assert("read", r == 2 && strcmp(buf, msg) == 0);
    close(sv[0]);
    close(sv[1]);
    return 0;
}

static const char *test_socket_addresses(void)
{
    int srv = socket(AF_INET, SOCK_STREAM, 0);
    mu_assert("srv socket", srv >= 0);

    struct sockaddr_in sa = {0};
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(0x7F000001);
    sa.sin_port = 0;
    mu_assert("bind", bind(srv, (struct sockaddr *)&sa, sizeof(sa)) == 0);
    mu_assert("listen", listen(srv, 1) == 0);

    struct sockaddr_in bound = {0};
    socklen_t blen = sizeof(bound);
    mu_assert("getsockname", getsockname(srv,
                                         (struct sockaddr *)&bound,
                                         &blen) == 0);
    mu_assert("family", bound.sin_family == AF_INET);

    int cli = socket(AF_INET, SOCK_STREAM, 0);
    mu_assert("cli socket", cli >= 0);
    mu_assert("connect", connect(cli, (struct sockaddr *)&bound,
                                 sizeof(bound)) == 0);

    int conn = accept(srv, NULL, NULL);
    mu_assert("accept", conn >= 0);

    struct sockaddr_in peer = {0};
    socklen_t plen = sizeof(peer);
    mu_assert("getpeername", getpeername(cli,
                                         (struct sockaddr *)&peer,
                                         &plen) == 0);
    mu_assert("peer", peer.sin_family == AF_INET);

    mu_assert("shutdown", shutdown(cli, SHUT_RDWR) == 0);

    close(conn);
    close(cli);
    close(srv);
    return 0;
}

static const char *test_dup3_cloexec(void)
{
    const char *fname = "tmp_dup3_file";
    int fd = open(fname, O_CREAT | O_RDWR, 0644);
    mu_assert("open", fd >= 0);
    int fd2 = dup3(fd, fd + 1, O_CLOEXEC);
    mu_assert("dup3", fd2 >= 0);
    close(fd);
    close(fd2);
    unlink(fname);
    return 0;
}

static const char *test_pipe2_cloexec(void)
{
    int p[2];
    mu_assert("pipe2", pipe2(p, O_CLOEXEC) == 0);
    close(p[0]);
    close(p[1]);
    return 0;
}

static const char *test_isatty_stdin(void)
{
    int fd = open("tmp_isatty_file", O_CREAT | O_RDWR, 0644);
    mu_assert("open", fd >= 0);
    int stdin_tty = isatty(0);
    int file_tty = isatty(fd);
    close(fd);
    unlink("tmp_isatty_file");
    mu_assert("file not tty", file_tty == 0);
    mu_assert("stdin result valid", stdin_tty == 0 || stdin_tty == 1);
    return 0;
}

static const char *test_ttyname_dev_tty(void)
{
    int fd = open("/dev/tty", O_RDWR);
    if (fd < 0)
        return 0; /* skip when not available */
    char buf[64];
    int r = ttyname_r(fd, buf, sizeof(buf));
    char *name = ttyname(fd);
    close(fd);
    mu_assert("ttyname_r", r == 0 && buf[0] != '\0');
    mu_assert("ttyname", name && name[0] != '\0');
    return 0;
}

static const char *test_ttyname_openpty(void)
{
    int m, s;
    char expect[64];
    mu_assert("openpty", openpty(&m, &s, expect, NULL, NULL) == 0);
    char buf[64];
    int r = ttyname_r(s, buf, sizeof(buf));
    char *name = ttyname(s);
    close(m);
    close(s);
    mu_assert("ttyname_r openpty", r == 0 && strcmp(buf, expect) == 0);
    mu_assert("ttyname openpty", name && strcmp(name, expect) == 0);
    return 0;
}

static const char *test_udp_send_recv(void)
{
    int s1 = socket(AF_INET, SOCK_DGRAM, 0);
    int s2 = socket(AF_INET, SOCK_DGRAM, 0);
    mu_assert("udp socket1", s1 >= 0);
    mu_assert("udp socket2", s2 >= 0);

    struct sockaddr_in a1 = {0};
    a1.sin_family = AF_INET;
    a1.sin_port = htons(12345);
    a1.sin_addr.s_addr = htonl(0x7F000001);
    mu_assert("bind1", bind(s1, (struct sockaddr *)&a1, sizeof(a1)) == 0);

    struct sockaddr_in a2 = {0};
    a2.sin_family = AF_INET;
    a2.sin_port = htons(12346);
    a2.sin_addr.s_addr = htonl(0x7F000001);
    mu_assert("bind2", bind(s2, (struct sockaddr *)&a2, sizeof(a2)) == 0);

    const char *msg = "udp";
    ssize_t sent = sendto(s1, msg, strlen(msg), 0,
                          (struct sockaddr *)&a2, sizeof(a2));
    mu_assert("sendto", sent == (ssize_t)strlen(msg));

    char buf[8] = {0};
    struct sockaddr_in src = {0};
    socklen_t slen = sizeof(src);
    ssize_t rec = recvfrom(s2, buf, sizeof(buf) - 1, 0,
                           (struct sockaddr *)&src, &slen);
    mu_assert("recvfrom", rec == (ssize_t)strlen(msg));
    mu_assert("udp content", strcmp(buf, msg) == 0);
    mu_assert("src port", ntohs(src.sin_port) == 12345);

    close(s1);
    close(s2);
    return 0;
}

static const char *test_sendmsg_recvmsg(void)
{
    int sv[2];
    mu_assert("socketpair", socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == 0);

    struct iovec wiov[2];
    wiov[0].iov_base = (void *)"he";
    wiov[0].iov_len = 2;
    wiov[1].iov_base = (void *)"llo";
    wiov[1].iov_len = 3;
    struct msghdr wmsg = {0};
    wmsg.msg_iov = wiov;
    wmsg.msg_iovlen = 2;
    mu_assert("sendmsg", sendmsg(sv[0], &wmsg, 0) == 5);

    char b1[3] = {0};
    char b2[4] = {0};
    struct iovec riov[2];
    riov[0].iov_base = b1;
    riov[0].iov_len = 2;
    riov[1].iov_base = b2;
    riov[1].iov_len = 3;
    struct msghdr rmsg = {0};
    rmsg.msg_iov = riov;
    rmsg.msg_iovlen = 2;
    ssize_t r = recvmsg(sv[1], &rmsg, 0);
    mu_assert("recvmsg", r == 5);
    mu_assert("content", strcmp(b1, "he") == 0 && strcmp(b2, "llo") == 0);

    close(sv[0]);
    close(sv[1]);
    return 0;
}

static const char *test_inet_pton_ntop(void)
{
    struct in_addr addr;
    int r = inet_pton(AF_INET, "127.2.3.4", &addr);
    mu_assert("inet_pton", r == 1);
    char buf[INET_ADDRSTRLEN];
    const char *p = inet_ntop(AF_INET, &addr, buf, sizeof(buf));
    mu_assert("inet_ntop", p && strcmp(buf, "127.2.3.4") == 0);
    struct in_addr back;
    r = inet_pton(AF_INET, buf, &back);
    mu_assert("inet_pton round", r == 1 && back.s_addr == addr.s_addr);

    struct in6_addr addr6;
    r = inet_pton(AF_INET6, "2001:db8::1", &addr6);
    mu_assert("inet_pton6", r == 1);
    char buf6[INET6_ADDRSTRLEN];
    p = inet_ntop(AF_INET6, &addr6, buf6, sizeof(buf6));
    mu_assert("inet_ntop6", p && strcmp(buf6, "2001:db8::1") == 0);
    struct in6_addr back6;
    r = inet_pton(AF_INET6, buf6, &back6);
    mu_assert("inet_pton6 round", r == 1 && memcmp(&back6, &addr6, sizeof(addr6)) == 0);
    return 0;
}

static const char *test_inet_aton_ntoa(void)
{
    struct in_addr addr;
    int r = inet_aton("192.0.2.5", &addr);
    mu_assert("inet_aton", r == 1);
    char *s = inet_ntoa(addr);
    mu_assert("inet_ntoa", strcmp(s, "192.0.2.5") == 0);
    struct in_addr back;
    r = inet_aton(s, &back);
    mu_assert("inet_aton round", r == 1 && back.s_addr == addr.s_addr);
    return 0;
}

static const char *test_errno_open(void)
{
    int fd = open("/this/file/does/not/exist", O_RDONLY);
    mu_assert("open should fail", fd == -1);
    mu_assert("errno should be ENOENT", errno == ENOENT);
    return 0;
}

static const char *test_errno_stat(void)
{
    struct stat st;
    int r = stat("/this/file/does/not/exist", &st);
    mu_assert("stat should fail", r == -1);
    mu_assert("errno should be ENOENT", errno == ENOENT);
    return 0;
}

static const char *test_stat_wrappers(void)
{
    const char *fname = "tmp_stat_file";
    int fd = open(fname, O_CREAT | O_RDWR, 0644);
    mu_assert("open failed", fd >= 0);
    const char *msg = "hello";
    ssize_t w = write(fd, msg, strlen(msg));
    mu_assert("write failed", w == (ssize_t)strlen(msg));
    close(fd);

    struct stat st;
    int r = stat(fname, &st);
    mu_assert("stat failed", r == 0);
    mu_assert("size mismatch", st.st_size == (off_t)strlen(msg));

    fd = open(fname, O_RDONLY);
    mu_assert("open2 failed", fd >= 0);
    r = fstat(fd, &st);
    mu_assert("fstat failed", r == 0);
    mu_assert("size mismatch", st.st_size == (off_t)strlen(msg));
    close(fd);

    r = lstat(fname, &st);
    mu_assert("lstat failed", r == 0);
    mu_assert("size mismatch", st.st_size == (off_t)strlen(msg));

    unlink(fname);
    return 0;
}

static const char *test_truncate_resize(void)
{
    const char *fname = "tmp_trunc_file";
    int fd = open(fname, O_CREAT | O_RDWR, 0644);
    mu_assert("open", fd >= 0);

    const char *msg = "abcdef";
    ssize_t w = write(fd, msg, strlen(msg));
    mu_assert("write", w == (ssize_t)strlen(msg));
    close(fd);

    int r = truncate(fname, 3);
    mu_assert("truncate", r == 0);

    struct stat st;
    r = stat(fname, &st);
    mu_assert("size shrink", r == 0 && st.st_size == 3);

    fd = open(fname, O_RDWR);
    mu_assert("open2", fd >= 0);
    r = ftruncate(fd, 10);
    mu_assert("ftruncate", r == 0);
    close(fd);

    r = stat(fname, &st);
    mu_assert("size expand", r == 0 && st.st_size == 10);

    unlink(fname);
    return 0;
}

static const char *test_posix_fallocate_basic(void)
{
    const char *fname = "tmp_pfall_file";
    int fd = open(fname, O_CREAT | O_RDWR, 0644);
    mu_assert("open", fd >= 0);

    int r = posix_fallocate(fd, 0, 8192);
    mu_assert("posix_fallocate", r == 0);

    struct stat st;
    r = fstat(fd, &st);
    mu_assert("size", r == 0 && st.st_size == 8192);
    close(fd);
    unlink(fname);
    return 0;
}

static const char *test_posix_fadvise_basic(void)
{
    const char *fname = "tmp_padvise_file";
    int fd = open(fname, O_CREAT | O_RDWR, 0644);
    mu_assert("open", fd >= 0);

    int r = posix_fadvise(fd, 0, 4096, POSIX_FADV_NORMAL);
    mu_assert("posix_fadvise", r == 0);

    close(fd);
    unlink(fname);
    return 0;
}

static const char *test_posix_fadvise_invalid(void)
{
    const char *fname = "tmp_padvise_file2";
    int fd = open(fname, O_CREAT | O_RDWR, 0644);
    mu_assert("open", fd >= 0);

    int r = posix_fadvise(fd, 0, 4096, -1);
#ifdef SYS_posix_fadvise
    mu_assert("posix_fadvise invalid", r == EINVAL);
#else
    mu_assert("posix_fadvise ignored", r == 0);
#endif

    close(fd);
    unlink(fname);
    return 0;
}

static const char *test_posix_madvise_basic(void)
{
    char buf[4096];
    int r = posix_madvise(buf, sizeof(buf), POSIX_MADV_NORMAL);
    mu_assert("posix_madvise", r == 0);
    return 0;
}

static const char *test_link_readlink(void)
{
    const char *target = "tmp_ln_target";
    const char *hard = "tmp_ln_hard";
    const char *sym = "tmp_ln_sym";

    int fd = open(target, O_CREAT | O_RDWR, 0644);
    mu_assert("open target", fd >= 0);
    write(fd, "x", 1);
    close(fd);

    mu_assert("link", link(target, hard) == 0);
    fd = open(hard, O_RDONLY);
    mu_assert("open hard", fd >= 0);
    char c;
    mu_assert("read hard", read(fd, &c, 1) == 1 && c == 'x');
    close(fd);

    mu_assert("symlink", symlink(target, sym) == 0);
    char buf[64] = {0};
    ssize_t n = readlink(sym, buf, sizeof(buf) - 1);
    mu_assert("readlink", n >= 0);
    buf[n] = '\0';
    mu_assert("link target", strcmp(buf, target) == 0);

    unlink(target);
    unlink(hard);
    unlink(sym);
    return 0;
}

static const char *test_at_wrappers_basic(void)
{
    const char *file = "tmp_at_file";
    const char *ln1  = "tmp_at_link";
    const char *ln2  = "tmp_at_link2";
    const char *node = "tmp_at_node";

    int dfd = open(".", O_RDONLY);
    mu_assert("open dir", dfd >= 0);

    int fd = openat(dfd, file, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    mu_assert("openat", fd >= 0);
    write(fd, "a", 1);
    close(fd);

    mu_assert("linkat", linkat(dfd, file, dfd, ln1, 0) == 0);
    mu_assert("renameat", renameat(dfd, ln1, dfd, ln2) == 0);
#ifndef S_IFREG
#define S_IFREG 0100000
#endif
    mu_assert("mknodat", mknodat(dfd, node, S_IFREG | 0600, 0) == 0);

    fd = openat(dfd, ln2, O_RDONLY);
    mu_assert("open renamed", fd >= 0);
    close(fd);
    fd = openat(dfd, node, O_RDONLY);
    mu_assert("open node", fd >= 0);
    close(fd);

    unlinkat(dfd, file, 0);
    unlinkat(dfd, ln2, 0);
    unlinkat(dfd, node, 0);
    close(dfd);
    return 0;
}

static const char *test_fsync_basic(void)
{
    const char *fname = "tmp_fsync_file";
    int fd = open(fname, O_CREAT | O_RDWR, 0644);
    mu_assert("open", fd >= 0);
    mu_assert("write", write(fd, "x", 1) == 1);
    int r = fsync(fd);
    close(fd);
    unlink(fname);
    mu_assert("fsync", r == 0);
    return 0;
}

static const char *test_fdatasync_basic(void)
{
    const char *fname = "tmp_fdatasync_file";
    int fd = open(fname, O_CREAT | O_RDWR, 0644);
    mu_assert("open", fd >= 0);
    mu_assert("write", write(fd, "y", 1) == 1);
    int r = fdatasync(fd);
    close(fd);
    unlink(fname);
    mu_assert("fdatasync", r == 0);
    return 0;
}

static const char *test_sync_basic(void)
{
    sync();
    return 0;
}

static const char *test_string_helpers(void)
{
    mu_assert("strcmp equal", strcmp("abc", "abc") == 0);
    mu_assert("strcmp lt", strcmp("abc", "abd") < 0);
    mu_assert("strcmp gt", strcmp("abd", "abc") > 0);

    const char *p = strchr("hello", 'e');
    mu_assert("strchr failed", p && p - "hello" == 1);

    char tmp[5];
    for (int i = 0; i < 5; ++i) tmp[i] = 'X';
    strncpy(tmp, "abc", 2);
    mu_assert("strncpy partial", tmp[0] == 'a' && tmp[1] == 'b' && tmp[2] == 'X');

    char buf[5];
    strncpy(buf, "hi", sizeof(buf));
    mu_assert("strncpy pad", buf[2] == '\0' && buf[3] == '\0');

    char *dup = strdup("test");
    mu_assert("strdup failed", dup && strcmp(dup, "test") == 0);
    free(dup);

    mu_assert("atoi", atoi("42") == 42);
    char *end;
    mu_assert("strtol hex", strtol("ff", &end, 16) == 255 && *end == '\0');
    mu_assert("strtol partial", strtol("12xy", &end, 10) == 12 && strcmp(end, "xy") == 0);
    mu_assert("strtoul basic", strtoul("123", &end, 10) == 123ul && *end == '\0');
    mu_assert("strtoll neg", strtoll("-321", &end, 10) == -321ll && *end == '\0');
    mu_assert("strtoull big", strtoull("1234567890123", &end, 10) == 1234567890123ull && *end == '\0');
    mu_assert("strtod basic", strtod("2.5", &end) == 2.5 && *end == '\0');
    mu_assert("strtod exp", strtod("1e2", &end) == 100.0 && *end == '\0');
    mu_assert("strtof", fabsf(strtof("4.5", &end) - 4.5f) < 1e-6f && *end == '\0');
    long double ld = strtold("6.25", &end);
    long double ldiff = ld - 6.25L;
    if (ldiff < 0)
        ldiff = -ldiff;
    mu_assert("strtold", ldiff < 1e-9L && *end == '\0');
    mu_assert("atof", atof("-3.0") == -3.0);

    char numbuf[64];
    snprintf(numbuf, sizeof(numbuf), "%jd", (intmax_t)INTMAX_MAX);
    mu_assert("strtoimax max",
              strtoimax(numbuf, &end, 10) == INTMAX_MAX && *end == '\0');
    snprintf(numbuf, sizeof(numbuf), "%jd", (intmax_t)INTMAX_MIN);
    mu_assert("strtoimax min",
              strtoimax(numbuf, &end, 10) == INTMAX_MIN && *end == '\0');
    snprintf(numbuf, sizeof(numbuf), "%ju", (uintmax_t)UINTMAX_MAX);
    mu_assert("strtoumax max",
              strtoumax(numbuf, &end, 10) == UINTMAX_MAX && *end == '\0');

    wchar_t wbuf[64];
    mbstowcs(wbuf, "ff", 64);
    wchar_t *wend;
    mu_assert("wcstol hex", wcstol(wbuf, &wend, 16) == 255 && *wend == L'\0');
    mbstowcs(wbuf, "123", 64);
    mu_assert("wcstoul basic", wcstoul(wbuf, &wend, 10) == 123ul && *wend == L'\0');
    mbstowcs(wbuf, "-321", 64);
    mu_assert("wcstoll neg", wcstoll(wbuf, &wend, 10) == -321ll && *wend == L'\0');
    mbstowcs(wbuf, "1234567890123", 64);
    mu_assert("wcstoull big", wcstoull(wbuf, &wend, 10) == 1234567890123ull && *wend == L'\0');
    mbstowcs(wbuf, "2.5", 64);
    mu_assert("wcstod basic", wcstod(wbuf, &wend) == 2.5 && *wend == L'\0');
    mbstowcs(wbuf, "4.5", 64);
    mu_assert("wcstof", fabsf(wcstof(wbuf, &wend) - 4.5f) < 1e-6f && *wend == L'\0');
    mbstowcs(wbuf, "6.25", 64);
    long double wld = wcstold(wbuf, &wend);
    long double wdiff = wld - 6.25L;
    if (wdiff < 0)
        wdiff = -wdiff;
    mu_assert("wcstold", wdiff < 1e-9L && *wend == L'\0');
    snprintf(numbuf, sizeof(numbuf), "%jd", (intmax_t)INTMAX_MAX);
    mbstowcs(wbuf, numbuf, 64);
    mu_assert("wcstoimax max",
              wcstoimax(wbuf, &wend, 10) == INTMAX_MAX && *wend == L'\0');
    snprintf(numbuf, sizeof(numbuf), "%jd", (intmax_t)INTMAX_MIN);
    mbstowcs(wbuf, numbuf, 64);
    mu_assert("wcstoimax min",
              wcstoimax(wbuf, &wend, 10) == INTMAX_MIN && *wend == L'\0');
    snprintf(numbuf, sizeof(numbuf), "%ju", (uintmax_t)UINTMAX_MAX);
    mbstowcs(wbuf, numbuf, 64);
    mu_assert("wcstoumax max",
              wcstoumax(wbuf, &wend, 10) == UINTMAX_MAX && *wend == L'\0');

    mu_assert("strnlen zero", strnlen("abc", 0) == 0);
    mu_assert("strnlen short", strnlen("hello", 3) == 3);
    mu_assert("strnlen full", strnlen("hi", 10) == 2);

    const char *h = "abcabc";
    const char *s = strstr(h, "cab");
    mu_assert("strstr", s && s - h == 2);

    const char *r = strrchr("abca", 'a');
    mu_assert("strrchr", r && r - "abca" == 3);

    char mbuf[4] = {1, 2, 3, 4};
    void *m = memchr(mbuf, 3, sizeof(mbuf));
    mu_assert("memchr", m == &mbuf[2]);
    mu_assert("memchr none", memchr(mbuf, 5, sizeof(mbuf)) == NULL);

    char rbuf[4] = {1, 2, 3, 2};
    void *mr = memrchr(rbuf, 2, sizeof(rbuf));
    mu_assert("memrchr", mr == &rbuf[3]);
    mu_assert("memrchr none", memrchr(rbuf, 5, sizeof(rbuf)) == NULL);

    const char hay[] = "abcdabcd";
    void *mm = memmem(hay, sizeof(hay) - 1, "cdab", 4);
    mu_assert("memmem mid", mm == hay + 2);
    mu_assert("memmem none", memmem(hay, sizeof(hay) - 1, "zz", 2) == NULL);
    mu_assert("memmem empty", memmem(hay, sizeof(hay) - 1, "", 0) == hay);

    mu_assert("strspn", strspn("abcde", "abc") == 3);
    mu_assert("strcspn", strcspn("hello world", " ") == 5);
    const char *bp = strpbrk("hello", "xol");
    mu_assert("strpbrk", bp && bp - "hello" == 2);

    return 0;
}

static const char *test_string_casecmp(void)
{
    mu_assert("strcasecmp eq", strcasecmp("HeLLo", "hello") == 0);
    mu_assert("strcasecmp diff", strcasecmp("abc", "Abd") < 0);
    mu_assert("strncasecmp n4", strncasecmp("TestX", "testY", 4) == 0);
    const char *p = strcasestr("AbcDe", "cde");
    mu_assert("strcasestr", p && p - "AbcDe" == 2);
    mu_assert("strcasestr none", strcasestr("abcd", "EF") == NULL);
    return 0;
}

static const char *test_strlcpy_cat(void)
{
    char buf[16];
    size_t r = strlcpy(buf, "abc", sizeof(buf));
    mu_assert("strlcpy ret", r == 3);
    mu_assert("strlcpy copy", strcmp(buf, "abc") == 0);

    char t[4];
    r = strlcpy(t, "abcdef", sizeof(t));
    mu_assert("strlcpy trunc ret", r == 6);
    mu_assert("strlcpy trunc", strcmp(t, "abc") == 0);

    char cbuf[10] = "foo";
    r = strlcat(cbuf, "bar", sizeof(cbuf));
    mu_assert("strlcat ret", r == 6);
    mu_assert("strlcat copy", strcmp(cbuf, "foobar") == 0);

    char c2[7] = "hello";
    r = strlcat(c2, "world", sizeof(c2));
    mu_assert("strlcat trunc ret", r == 10);
    mu_assert("strlcat trunc", strcmp(c2, "hellow") == 0);

    return 0;
}

static const char *test_stpcpy_functions(void)
{
    char buf[8];
    char *p = stpcpy(buf, "hi");
    mu_assert("stpcpy end", p == buf + 2);
    mu_assert("stpcpy copy", strcmp(buf, "hi") == 0);

    char buf2[6];
    p = stpncpy(buf2, "hello", 5);
    mu_assert("stpncpy end", p == buf2 + 5);
    mu_assert("stpncpy copy", strcmp(buf2, "hello") == 0);

    char buf3[4];
    p = stpncpy(buf3, "xyz123", 3);
    mu_assert("stpncpy trunc", p == buf3 + 3);
    mu_assert("stpncpy trunc str", strncmp(buf3, "xyz", 3) == 0);

    return 0;
}

static const char *test_memccpy_mempcpy(void)
{
    char src[] = "abcde";
    char buf[6] = {0};
    char *p = memccpy(buf, src, 'c', sizeof(src));
    mu_assert("memccpy ptr", p == buf + 3);
    mu_assert("memccpy copy", buf[0] == 'a' && buf[1] == 'b' && buf[2] == 'c');

    char dst[5];
    char *end = mempcpy(dst, "wxyz", 4);
    mu_assert("mempcpy end", end == dst + 4);
    mu_assert("mempcpy copy", memcmp(dst, "wxyz", 4) == 0);

    char other[4] = {0};
    p = memccpy(other, src, 'z', 4);
    mu_assert("memccpy not found", p == NULL && memcmp(other, "abcd", 4) == 0);

    return 0;
}

static const char *test_memccpy_zero(void)
{
    char dst[4] = {1, 2, 3, 4};
    char orig[4];
    memcpy(orig, dst, sizeof(dst));
    char *p = memccpy(dst, "xx", 'x', 0);
    mu_assert("memccpy zero", p == NULL && memcmp(dst, orig, sizeof(dst)) == 0);
    return 0;
}

static const char *test_strndup_basic(void)
{
    char *p = strndup("hello", 10);
    mu_assert("strndup copy", p && strcmp(p, "hello") == 0);
    free(p);

    p = strndup("truncate", 4);
    mu_assert("strndup trunc", p && strcmp(p, "trun") == 0);
    free(p);

    return 0;
}

static const char *test_strcoll_xfrm(void)
{
    mu_assert("strcoll eq", strcoll("abc", "abc") == 0);
    mu_assert("strcoll lt", strcoll("abc", "abd") < 0);
    char buf[8];
    size_t n = strxfrm(buf, "abc", sizeof(buf));
    mu_assert("strxfrm len", n == 3);
    mu_assert("strxfrm copy", strcmp(buf, "abc") == 0);
    return 0;
}

static const char *test_wcscoll_xfrm(void)
{
    mu_assert("wcscoll eq", wcscoll(L"abc", L"abc") == 0);
    mu_assert("wcscoll lt", wcscoll(L"abc", L"abd") < 0);
    wchar_t buf[8];
    size_t n = wcsxfrm(buf, L"abc", sizeof(buf) / sizeof(wchar_t));
    mu_assert("wcsxfrm len", n == 3);
    mu_assert("wcsxfrm copy", wcsncmp(buf, L"abc", 4) == 0);
    return 0;
}

static const char *test_ctype_extra(void)
{
    mu_assert("isprint", isprint('A'));
    mu_assert("isprint space", isprint(' '));
    mu_assert("iscntrl", iscntrl('\n'));
    mu_assert("ispunct", ispunct('!'));
    mu_assert("isgraph", isgraph('!'));
    mu_assert("!isgraph space", !isgraph(' '));
    mu_assert("isblank space", isblank(' '));
    mu_assert("isblank tab", isblank('\t'));
    mu_assert("!isblank nl", !isblank('\n'));
    return 0;
}

static const char *test_widechar_basic(void)
{
    wchar_t wc = 0;
    mu_assert("mbtowc ascii", mbtowc(&wc, "A", 1) == 1 && wc == L'A');
    char buf[2] = {0};
    mu_assert("wctomb ascii", wctomb(buf, wc) == 1 && buf[0] == 'A');
    mu_assert("wcslen", wcslen(L"abc") == 3);
    mu_assert("mbtowc reset", mbtowc(NULL, NULL, 0) == 0);
    return 0;
}

static const char *test_widechar_conv(void)
{
    wchar_t wbuf[4];
    size_t n = mbstowcs(wbuf, "abc", 4);
    mu_assert("mbstowcs count", n == 3);
    mu_assert("mbstowcs conv", wbuf[0] == L'a' && wbuf[1] == L'b' && wbuf[2] == L'c');

    char mbuf[4];
    n = wcstombs(mbuf, wbuf, 4);
    mu_assert("wcstombs count", n == 3);
    mu_assert("wcstombs conv", strcmp(mbuf, "abc") == 0);

    mbstate_t st = {0};
    n = mbrlen("z", 1, &st);
    mu_assert("mbrlen", n == 1);

    wchar_t wc = 0;
    n = mbrtowc(&wc, "x", 1, &st);
    mu_assert("mbrtowc", n == 1 && wc == L'x');

    char out[2] = {0};
    n = wcrtomb(out, wc, &st);
    mu_assert("wcrtomb", n == 1 && out[0] == 'x');

    mu_assert("mbsinit", mbsinit(&st));

    const char *m = "h\xC3\xA9llo";
    const char *mp = m;
    wchar_t wtmp[8];
    n = mbsrtowcs(wtmp, &mp, 8, &st);
    mu_assert("mbsrtowcs len", n == 5 && mp == NULL);

    const wchar_t *wp = wtmp;
    char mbtmp[16] = {0};
    mbstate_t st2 = {0};
    size_t nb = wcsrtombs(mbtmp, &wp, sizeof(mbtmp), &st2);
    mu_assert("wcsrtombs len", nb == strlen(m));
    mu_assert("wcsrtombs end", wp == NULL);
    mu_assert("roundtrip", strcmp(mbtmp, m) == 0);

    return 0;
}

static const char *test_widechar_width(void)
{
    mu_assert("wcwidth ascii", wcwidth(L'A') == 1);
    mu_assert("wcwidth nul", wcwidth(L'\0') == 0);
    mu_assert("wcwidth ctrl", wcwidth(L'\n') == -1);
    mu_assert("wcswidth", wcswidth(L"hi", 2) == 2);
    return 0;
}

static const char *test_wctype_checks(void)
{
    mu_assert("iswalpha", iswalpha(L'A'));
    mu_assert("iswdigit", iswdigit(L'5'));
    mu_assert("iswalnum", iswalnum(L'9'));
    mu_assert("iswspace", iswspace(L' '));
    mu_assert("iswupper", iswupper(L'A'));
    mu_assert("iswlower", iswlower(L'z'));
    mu_assert("iswxdigit", iswxdigit(L'F'));
    mu_assert("iswprint", iswprint(L'!'));
    mu_assert("iswcntrl", iswcntrl(L'\n'));
    mu_assert("iswpunct", iswpunct(L'!'));
    mu_assert("iswgraph", iswgraph(L'!'));
    mu_assert("iswblank", iswblank(L'\t'));
    mu_assert("towlower", towlower(L'A') == L'a');
    mu_assert("towupper", towupper(L'a') == L'A');
    return 0;
}

static const char *test_wmem_ops(void)
{
    wchar_t buf[8];
    wmemset(buf, L'x', 8);
    for (size_t i = 0; i < 8; i++)
        mu_assert("wmemset", buf[i] == L'x');

    wchar_t src[8] = { L'a', L'b', L'c', L'd', L'e', L'f', L'g', L'h' };
    wmemcpy(buf, src, 8);
    mu_assert("wmemcpy", wmemcmp(buf, src, 8) == 0);

    wmemmove(buf + 1, buf, 7);
    mu_assert("wmemmove", buf[1] == L'a' && buf[2] == L'b');

    wchar_t a[] = { L'a', L'b', L'c' };
    wchar_t b[] = { L'a', L'b', L'd' };
    mu_assert("wmemcmp diff", wmemcmp(a, b, 3) < 0);

    return 0;
}

static const char *test_wchar_search(void)
{
    const wchar_t *p = wcschr(L"hello", L'e');
    mu_assert("wcschr", p && p - L"hello" == 1);

    const wchar_t *r = wcsrchr(L"abca", L'a');
    mu_assert("wcsrchr", r && r - L"abca" == 3);

    const wchar_t *h = L"abcabc";
    const wchar_t *s = wcsstr(h, L"cab");
    mu_assert("wcsstr", s && s - h == 2);

    wchar_t buf[4] = { L'x', L'y', L'z', L'y' };
    wchar_t *m = wmemchr(buf, L'z', 4);
    mu_assert("wmemchr", m == &buf[2]);
    mu_assert("wmemchr none", wmemchr(buf, L'a', 4) == NULL);

    return 0;
}

static const char *test_wmemstream_basic(void)
{
    wchar_t *out = NULL;
    size_t len = 0;
    FILE *f = open_wmemstream(&out, &len);
    mu_assert("open_wmemstream", f != NULL);
    mu_assert("fwprintf", fwprintf(f, L"%ls %d", L"wide", 42) > 0);
    fclose(f);
    mu_assert("wmem len", len == 7);
    mu_assert("wmem content", out && wcsncmp(out, L"wide 42", len) == 0);
    free(out);
    return 0;
}

struct cookie_buf {
    char buf[64];
    size_t pos;
    size_t len;
    int read_called;
    int write_called;
};

static ssize_t cb_read(void *c, char *b, size_t n)
{
    struct cookie_buf *cb = c;
    if (cb->pos >= cb->len)
        return 0;
    size_t avail = cb->len - cb->pos;
    if (n > avail)
        n = avail;
    memcpy(b, cb->buf + cb->pos, n);
    cb->pos += n;
    cb->read_called++;
    return (ssize_t)n;
}

static ssize_t cb_write(void *c, const char *b, size_t n)
{
    struct cookie_buf *cb = c;
    if (cb->pos + n > sizeof(cb->buf))
        n = sizeof(cb->buf) - cb->pos;
    memcpy(cb->buf + cb->pos, b, n);
    cb->pos += n;
    if (cb->pos > cb->len)
        cb->len = cb->pos;
    cb->write_called++;
    return (ssize_t)n;
}

static int cb_seek(void *c, off_t *off, int whence)
{
    struct cookie_buf *cb = c;
    off_t newpos;
    if (whence == SEEK_SET)
        newpos = *off;
    else if (whence == SEEK_CUR)
        newpos = (off_t)cb->pos + *off;
    else if (whence == SEEK_END)
        newpos = (off_t)cb->len + *off;
    else
        return -1;
    if (newpos < 0 || (size_t)newpos > cb->len)
        return -1;
    cb->pos = (size_t)newpos;
    *off = newpos;
    return 0;
}

static const char *test_fopencookie_basic(void)
{
    struct cookie_buf cb = {0};
    cookie_io_functions_t io = { cb_read, cb_write, cb_seek, NULL };
    FILE *f = fopencookie(&cb, "w+", io);
    mu_assert("fopencookie", f != NULL);
    mu_assert("write", fwrite("abc", 1, 3, f) == 3);
    mu_assert("write_called", cb.write_called > 0);
    rewind(f);
    char out[4] = {0};
    mu_assert("read", fread(out, 1, 3, f) == 3);
    mu_assert("content", memcmp(out, "abc", 3) == 0);
    mu_assert("read_called", cb.read_called > 0);
    fclose(f);
    return 0;
}

static const char *test_iconv_ascii_roundtrip(void)
{
    iconv_t cd = iconv_open("UTF-8", "ASCII");
    mu_assert("iconv open", cd != (iconv_t)-1);
    char inbuf[] = "abc";
    char *in = inbuf;
    size_t inleft = 3;
    char out[8] = {0};
    char *outp = out;
    size_t outleft = sizeof(out);
    size_t r = iconv(cd, &in, &inleft, &outp, &outleft);
    mu_assert("iconv ok", r != (size_t)-1 && strcmp(out, "abc") == 0);
    mu_assert("iconv all consumed", inleft == 0);
    iconv_close(cd);
    return 0;
}

static const char *test_iconv_invalid_byte(void)
{
    iconv_t cd = iconv_open("ASCII", "UTF-8");
    mu_assert("iconv open2", cd != (iconv_t)-1);
    char in[2] = { (char)0xC3, (char)0x81 }; /* \xC3\x81 = U+00C1 */
    char *pin = in;
    size_t inleft = 2;
    char out[4];
    char *pout = out;
    size_t outleft = sizeof(out);
    size_t r = iconv(cd, &pin, &inleft, &pout, &outleft);
    mu_assert("iconv bad", r == (size_t)-1);
    iconv_close(cd);
    return 0;
}

static const char *test_strtok_basic(void)
{
    char buf[] = "a,b,c";
    char *tok = strtok(buf, ",");
    mu_assert("tok1", tok && strcmp(tok, "a") == 0);
    tok = strtok(NULL, ",");
    mu_assert("tok2", tok && strcmp(tok, "b") == 0);
    tok = strtok(NULL, ",");
    mu_assert("tok3", tok && strcmp(tok, "c") == 0);
    tok = strtok(NULL, ",");
    mu_assert("tok end", tok == NULL);
    return 0;
}

static const char *test_strtok_r_basic(void)
{
    char buf[] = "1 2 3";
    char *save = NULL;
    char *tok = strtok_r(buf, " ", &save);
    mu_assert("tok_r1", tok && strcmp(tok, "1") == 0);
    tok = strtok_r(NULL, " ", &save);
    mu_assert("tok_r2", tok && strcmp(tok, "2") == 0);
    tok = strtok_r(NULL, " ", &save);
    mu_assert("tok_r3", tok && strcmp(tok, "3") == 0);
    tok = strtok_r(NULL, " ", &save);
    mu_assert("tok_r end", tok == NULL);
    return 0;
}

static const char *test_strsep_basic(void)
{
    char buf[] = "x:y:z";
    char *p = buf;
    char *tok = strsep(&p, ":");
    mu_assert("sep1", tok && strcmp(tok, "x") == 0);
    tok = strsep(&p, ":");
    mu_assert("sep2", tok && strcmp(tok, "y") == 0);
    tok = strsep(&p, ":");
    mu_assert("sep3", tok && strcmp(tok, "z") == 0);
    tok = strsep(&p, ":");
    mu_assert("sep end", tok == NULL);
    return 0;
}

static const char *test_wcstok_basic(void)
{
    wchar_t buf[] = L"a b c";
    wchar_t *save = NULL;
    wchar_t *tok = wcstok(buf, L" ", &save);
    mu_assert("wcstok1", tok && wcscmp(tok, L"a") == 0);
    tok = wcstok(NULL, L" ", &save);
    mu_assert("wcstok2", tok && wcscmp(tok, L"b") == 0);
    tok = wcstok(NULL, L" ", &save);
    mu_assert("wcstok3", tok && wcscmp(tok, L"c") == 0);
    tok = wcstok(NULL, L" ", &save);
    mu_assert("wcstok end", tok == NULL);
    return 0;
}

static const char *test_printf_functions(void)
{
    char buf[64];
    int n = snprintf(buf, sizeof(buf), "v=%d %s", 42, "ok");
    mu_assert("snprintf len", n == (int)strlen("v=42 ok"));
    mu_assert("snprintf buf", strcmp(buf, "v=42 ok") == 0);

    n = snprintf(buf, sizeof(buf), "%X %o %c", 0x2B, 10, 'A');
    mu_assert("hex/oct/char", strcmp(buf, "2B 12 A") == 0);

    n = snprintf(buf, sizeof(buf), "%x", 0x2b);
    mu_assert("lower hex", strcmp(buf, "2b") == 0);

    n = snprintf(buf, sizeof(buf), "[%4o]", 9);
    mu_assert("octal width", strcmp(buf, "[   11]") == 0);

    n = snprintf(buf, sizeof(buf), "[%.3o]", 7);
    mu_assert("octal precision", strcmp(buf, "[007]") == 0);

    int x = 0;
    n = snprintf(buf, sizeof(buf), "%p", &x);
    mu_assert("pointer prefix", strncmp(buf, "0x", 2) == 0);

    n = snprintf(buf, sizeof(buf), "[%5x]", 1);
    mu_assert("field width", strcmp(buf, "[    1]") == 0);

    n = snprintf(buf, sizeof(buf), "[%.4x]", 3);
    mu_assert("precision", strcmp(buf, "[0003]") == 0);

    FILE *f = fopen("tmp_pf", "w");
    mu_assert("fopen failed", f != NULL);
    fprintf(f, "num=%d", 7);
    fclose(f);

    int fd = open("tmp_pf", O_RDONLY);
    char rbuf[16] = {0};
    ssize_t r = read(fd, rbuf, sizeof(rbuf) - 1);
    close(fd);
    unlink("tmp_pf");
    mu_assert("fprintf read", r > 0);
    mu_assert("fprintf content", strncmp(rbuf, "num=7", 5) == 0);

    printf("printf check %u\n", 123u);

    return 0;
}

static int call_vdprintf(int fd, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int r = vdprintf(fd, fmt, ap);
    va_end(ap);
    return r;
}

static const char *test_dprintf_functions(void)
{
    int fd = open("tmp_dpr", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    mu_assert("open dprintf", fd >= 0);
    dprintf(fd, "val=%d", 5);
    close(fd);

    char buf[16] = {0};
    fd = open("tmp_dpr", O_RDONLY);
    ssize_t r = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    unlink("tmp_dpr");
    mu_assert("dprintf read", r > 0);
    mu_assert("dprintf content", strcmp(buf, "val=5") == 0);

    fd = open("tmp_vdpr", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    mu_assert("open vdprintf", fd >= 0);
    call_vdprintf(fd, "num=%u", 10u);
    close(fd);

    char buf2[16] = {0};
    fd = open("tmp_vdpr", O_RDONLY);
    r = read(fd, buf2, sizeof(buf2) - 1);
    close(fd);
    unlink("tmp_vdpr");
    mu_assert("vdprintf read", r > 0);
    mu_assert("vdprintf content", strcmp(buf2, "num=10") == 0);

    return 0;
}

static const char *test_scanf_functions(void)
{
    vlibc_init();

    int a = 0;
    unsigned b = 0;
    char str[16] = {0};
    int r = sscanf("5 10 test", "%d %u %s", &a, &b, str);
    mu_assert("sscanf count", r == 3);
    mu_assert("sscanf a", a == 5);
    mu_assert("sscanf b", b == 10);
    mu_assert("sscanf str", strcmp(str, "test") == 0);

    FILE *f = fopen("tmp_scan", "w+");
    mu_assert("fopen scan", f != NULL);
    fputs("7 8 hi", f);
    rewind(f);
    a = b = 0; str[0] = '\0';
    r = fscanf(f, "%d %u %s", &a, &b, str);
    fclose(f);
    unlink("tmp_scan");
    mu_assert("fscanf count", r == 3);
    mu_assert("fscanf a", a == 7);
    mu_assert("fscanf b", b == 8);
    mu_assert("fscanf str", strcmp(str, "hi") == 0);

    f = fopen("tmp_hex", "w+");
    mu_assert("fopen hex", f != NULL);
    fputs("ff 12", f);
    rewind(f);
    a = b = 0;
    r = fscanf(f, "%x %o", &a, &b);
    fclose(f);
    unlink("tmp_hex");
    mu_assert("fscanf hex count", r == 2);
    mu_assert("fscanf hex val", a == 0xff);
    mu_assert("fscanf oct val", b == 10u);

    int p[2];
    mu_assert("pipe", pipe(p) == 0);
    write(p[1], "9 11 end", 9);
    close(p[1]);
    int old = stdin->fd;
    stdin->fd = p[0];
    a = b = 0; str[0] = '\0';
    r = scanf("%d %u %s", &a, &b, str);
    stdin->fd = old;
    close(p[0]);
    mu_assert("scanf count", r == 3);
    mu_assert("scanf a", a == 9);
    mu_assert("scanf b", b == 11);
    mu_assert("scanf str", strcmp(str, "end") == 0);

    mu_assert("pipe", pipe(p) == 0);
    write(p[1], "1a 17", 5);
    close(p[1]);
    old = stdin->fd;
    stdin->fd = p[0];
    a = b = 0;
    r = scanf("%x %o", &a, &b);
    stdin->fd = old;
    close(p[0]);
    mu_assert("scanf hex count", r == 2);
    mu_assert("scanf hex val", a == 0x1a);
    mu_assert("scanf oct val", b == 015);

    r = sscanf("ff 12", "%x %o", &a, &b);
    mu_assert("hex/octal count", r == 2);
    mu_assert("hex value", a == 0xff);
    mu_assert("oct value", b == 10u);

    float fv = 0.0f;
    double dv = 0.0;
    r = sscanf("3.5 4.25", "%f %lf", &fv, &dv);
    mu_assert("sscanf float count", r == 2);
    mu_assert("sscanf float val", fabs(fv - 3.5f) < 1e-6);
    mu_assert("sscanf double val", fabs(dv - 4.25) < 1e-9);

    f = fopen("tmp_fscan", "w+");
    mu_assert("fopen float", f != NULL);
    fputs("5.5 6.75", f);
    rewind(f);
    fv = 0.0f; dv = 0.0;
    r = fscanf(f, "%f %lg", &fv, &dv);
    fclose(f);
    unlink("tmp_fscan");
    mu_assert("fscanf float count", r == 2);
    mu_assert("fscanf float val", fabs(fv - 5.5f) < 1e-6);
    mu_assert("fscanf double val", fabs(dv - 6.75) < 1e-9);

    return 0;
}

static int call_vsscanf(const char *buf, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int r = vsscanf(buf, fmt, ap);
    va_end(ap);
    return r;
}

static int call_vfscanf(FILE *f, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int r = vfscanf(f, fmt, ap);
    va_end(ap);
    return r;
}


static const char *test_vscanf_variants(void)
{
    vlibc_init();

    int a = 0;
    unsigned b = 0;
    char str[16] = {0};

    int r = call_vsscanf("4 5 buf", "%d %u %s", &a, &b, str);
    mu_assert("vsscanf count", r == 3);
    mu_assert("vsscanf a", a == 4);
    mu_assert("vsscanf b", b == 5);
    mu_assert("vsscanf str", strcmp(str, "buf") == 0);

    FILE *f = fopen("tmp_vscan", "w+");
    mu_assert("vfopen", f != NULL);
    fputs("6 7 file", f);
    rewind(f);
    a = b = 0; str[0] = '\0';
    r = call_vfscanf(f, "%d %u %s", &a, &b, str);
    fclose(f);
    unlink("tmp_vscan");
    mu_assert("vfscanf count", r == 3);
    mu_assert("vfscanf a", a == 6);
    mu_assert("vfscanf b", b == 7);
    mu_assert("vfscanf str", strcmp(str, "file") == 0);

    r = call_vsscanf("1a 17", "%x %o", &a, &b);
    mu_assert("vsscanf hex count", r == 2);
    mu_assert("vsscanf hex val", a == 0x1a);
    mu_assert("vsscanf oct val", b == 015);

    f = fopen("tmp_vscan2", "w+");
    mu_assert("vfopen2", f != NULL);
    fputs("ff 12", f);
    rewind(f);
    a = b = 0;
    r = call_vfscanf(f, "%x %o", &a, &b);
    fclose(f);
    unlink("tmp_vscan2");
    mu_assert("vfscanf hex count", r == 2);
    mu_assert("vfscanf hex val", a == 0xff);
    mu_assert("vfscanf oct val", b == 10u);

    float fv = 0.0f;
    double dv = 0.0;
    r = call_vsscanf("8.5 9.5", "%f %lg", &fv, &dv);
    mu_assert("vsscanf float count", r == 2);
    mu_assert("vsscanf float val", fabs(fv - 8.5f) < 1e-6);
    mu_assert("vsscanf double val", fabs(dv - 9.5) < 1e-9);

    f = fopen("tmp_vscan3", "w+");
    mu_assert("vfopen3", f != NULL);
    fputs("1.25 2.75", f);
    rewind(f);
    fv = 0.0f; dv = 0.0;
    r = call_vfscanf(f, "%f %lf", &fv, &dv);
    fclose(f);
    unlink("tmp_vscan3");
    mu_assert("vfscanf float count", r == 2);
    mu_assert("vfscanf float val", fabs(fv - 1.25f) < 1e-6);
    mu_assert("vfscanf double val", fabs(dv - 2.75) < 1e-9);

    return 0;
}

static const char *test_fseek_rewind(void)
{
    FILE *f = fopen("tmp_seek", "w+");
    mu_assert("fopen seek", f != NULL);

    const char *msg = "abcdef";
    size_t w = fwrite(msg, 1, strlen(msg), f);
    mu_assert("fwrite seek", w == strlen(msg));

    mu_assert("fseek set", fseek(f, 0, SEEK_SET) == 0);
    char buf[4] = {0};
    size_t r = fread(buf, 1, 3, f);
    mu_assert("fread seek", r == 3);
    mu_assert("content seek", strncmp(buf, "abc", 3) == 0);

    long pos = ftell(f);
    mu_assert("ftell pos", pos == 3);

    mu_assert("fseek end", fseek(f, 0, SEEK_END) == 0);
    pos = ftell(f);
    mu_assert("ftell end", pos == (long)strlen(msg));

    rewind(f);
    mu_assert("rewind pos", ftell(f) == 0);

    fclose(f);
    unlink("tmp_seek");
    return 0;
}

static const char *test_fgetpos_fsetpos(void)
{
    FILE *f = fopen("tmp_fpos", "w+");
    mu_assert("fopen fpos", f != NULL);

    const char *msg = "abcdef";
    size_t w = fwrite(msg, 1, strlen(msg), f);
    mu_assert("fwrite fpos", w == strlen(msg));

    rewind(f);
    char buf[4] = {0};
    size_t r = fread(buf, 1, 3, f);
    mu_assert("fread first", r == 3);
    mu_assert("content first", strncmp(buf, "abc", 3) == 0);

    fpos_t pos;
    mu_assert("fgetpos ret", fgetpos(f, &pos) == 0);

    r = fread(buf, 1, 3, f);
    mu_assert("fread second", r == 3);
    mu_assert("content second", strncmp(buf, "def", 3) == 0);

    mu_assert("fsetpos ret", fsetpos(f, &pos) == 0);
    memset(buf, 0, sizeof(buf));
    r = fread(buf, 1, 3, f);
    mu_assert("fread restore", r == 3);
    mu_assert("content restore", strncmp(buf, "def", 3) == 0);

    fclose(f);
    unlink("tmp_fpos");
    return 0;
}

static const char *test_fgetc_fputc(void)
{
    FILE *f = fopen("tmp_char", "w+");
    mu_assert("fopen char", f != NULL);
    mu_assert("fputc ret", fputc('X', f) == 'X');
    rewind(f);
    int c = fgetc(f);
    mu_assert("fgetc val", c == 'X');
    fclose(f);
    unlink("tmp_char");
    return 0;
}

static const char *test_fgets_fputs(void)
{
    FILE *f = fopen("tmp_line", "w+");
    mu_assert("fopen line", f != NULL);
    mu_assert("fputs ret", fputs("hello\n", f) >= 0);
    rewind(f);
    char buf[16] = {0};
    char *r = fgets(buf, sizeof(buf), f);
    mu_assert("fgets not null", r != NULL);
    mu_assert("fgets content", strcmp(buf, "hello\n") == 0);
    fclose(f);
    unlink("tmp_line");
    return 0;
}

static const char *test_fgetwc_fputwc(void)
{
    FILE *f = fopen("tmp_wcs", "w+");
    mu_assert("fopen wcs", f != NULL);
    mu_assert("fputwc ret", fputwc(L'Z', f) == L'Z');
    rewind(f);
    wint_t wc = fgetwc(f);
    mu_assert("fgetwc val", wc == L'Z');
    fclose(f);
    unlink("tmp_wcs");
    return 0;
}

static const char *test_getwc_putwc(void)
{
    FILE *f = fopen("tmp_wcs2", "w+");
    mu_assert("fopen wcs2", f != NULL);
    mu_assert("putwc ret", putwc(L'A', f) == L'A');
    rewind(f);
    wint_t wc = getwc(f);
    mu_assert("getwc val", wc == L'A');
    fclose(f);
    unlink("tmp_wcs2");
    return 0;
}

static const char *test_getline_various(void)
{
    FILE *f = fopen("tmp_getline", "w+");
    mu_assert("fopen", f != NULL);
    const char *content = "short\nlonger line here\nlast";
    mu_assert("write", fwrite(content, 1, strlen(content), f) == (ssize_t)strlen(content));
    rewind(f);
    char *line = NULL;
    size_t cap = 0;
    ssize_t len = getline(&line, &cap, f);
    mu_assert("line1", len == 6 && strcmp(line, "short\n") == 0);
    len = getline(&line, &cap, f);
    mu_assert("line2", len == 17 && strcmp(line, "longer line here\n") == 0);
    len = getline(&line, &cap, f);
    mu_assert("line3", len == 4 && strcmp(line, "last") == 0);
    len = getline(&line, &cap, f);
    mu_assert("eof", len == -1);
    free(line);
    fclose(f);
    unlink("tmp_getline");
    return 0;
}

static const char *test_getdelim_various(void)
{
    FILE *f = fopen("tmp_getdelim", "w+");
    mu_assert("fopen", f != NULL);
    const char *content = "aa,bbb,cccc,";
    mu_assert("write", fwrite(content, 1, strlen(content), f) == (ssize_t)strlen(content));
    rewind(f);
    char *line = NULL;
    size_t cap = 0;
    ssize_t len = getdelim(&line, &cap, ',', f);
    mu_assert("tok1", len == 3 && strcmp(line, "aa,") == 0);
    len = getdelim(&line, &cap, ',', f);
    mu_assert("tok2", len == 4 && strcmp(line, "bbb,") == 0);
    len = getdelim(&line, &cap, ',', f);
    mu_assert("tok3", len == 5 && strcmp(line, "cccc,") == 0);
    len = getdelim(&line, &cap, ',', f);
    mu_assert("eof", len == -1);
    free(line);
    fclose(f);
    unlink("tmp_getdelim");
    return 0;
}

static const char *test_fflush(void)
{
    FILE *f = fopen("tmp_flush", "w");
    mu_assert("fopen flush", f != NULL);
    mu_assert("write", fwrite("abc", 1, 3, f) == 3);
    mu_assert("fflush", fflush(f) == 0);
    fclose(f);

    int fd = open("tmp_flush", O_RDONLY);
    char buf[4] = {0};
    ssize_t r = read(fd, buf, 3);
    close(fd);
    unlink("tmp_flush");
    mu_assert("fflush content", r == 3 && strncmp(buf, "abc", 3) == 0);
    return 0;
}

static const char *test_feof_flag(void)
{
    FILE *f = fopen("tmp_feof", "w+");
    mu_assert("fopen", f != NULL);
    fwrite("abc", 1, 3, f);
    rewind(f);
    char buf[8];
    size_t n = fread(buf, 1, sizeof(buf), f);
    mu_assert("read count", n == 3);
    mu_assert("feof set", feof(f));
    mu_assert("no error", !ferror(f));
    n = fread(buf, 1, 1, f);
    mu_assert("read after eof", n == 0 && feof(f));
    fclose(f);
    unlink("tmp_feof");
    return 0;
}

static const char *test_ferror_flag(void)
{
    FILE *f = fopen("tmp_ferr", "w");
    mu_assert("create", f != NULL);
    fclose(f);
    f = fopen("tmp_ferr", "r");
    mu_assert("open", f != NULL);
    size_t w = fwrite("x", 1, 1, f);
    mu_assert("write fail", w == 0);
    mu_assert("ferror set", ferror(f));
    clearerr(f);
    mu_assert("clearerr", !ferror(f) && !feof(f));
    fclose(f);
    unlink("tmp_ferr");
    return 0;
}

static void *basic_worker(void *arg)
{
    *(int *)arg = 7;
    return NULL;
}

static const char *test_pthread_create_join(void)
{
    pthread_t t;
    int v = 0;
    mu_assert("create", pthread_create(&t, NULL, basic_worker, &v) == 0);
    mu_assert("join", pthread_join(t, NULL) == 0);
    mu_assert("value", v == 7);
    return 0;
}

static const char *test_pthread(void)
{
    pthread_t t;
    int val = 0;
    int r = pthread_create(&t, NULL, thread_fn, &val);
    mu_assert("pthread_create", r == 0);
    void *ret = NULL;
    pthread_join(t, &ret);
    mu_assert("thread retval", ret == (void *)123);
    mu_assert("shared value", val == 42);
  
    return 0;
}

static const char *test_pthread_detach(void)
{
    pthread_t t;
    int val = 0;
    int r = pthread_create(&t, NULL, thread_fn, &val);
    mu_assert("pthread_create", r == 0);
    pthread_detach(t);
    usleep(100000);
    mu_assert("shared value", val == 42);
    mu_assert("join fails", pthread_join(t, NULL) != 0);

    return 0;
}

static void *exit_worker(void *arg)
{
    pthread_exit(arg);
    return NULL;
}

static const char *test_pthread_exit(void)
{
    pthread_t t;
    int value = 55;
    pthread_create(&t, NULL, exit_worker, &value);
    void *ret = NULL;
    pthread_join(t, &ret);
    mu_assert("exit retval", ret == &value);
    return 0;
}

static void *cancel_worker(void *arg)
{
    (void)arg;
    for (;;)
        usleep(1000);
    return NULL;
}

static const char *test_pthread_cancel(void)
{
    pthread_t t;
    pthread_create(&t, NULL, cancel_worker, NULL);
    usleep(10000);
    pthread_cancel(t);
    void *ret = NULL;
    pthread_join(t, &ret);
    mu_assert("canceled", ret == PTHREAD_CANCELED);
    return 0;
}

static pthread_key_t tls_key;
static pthread_once_t once_ctl = PTHREAD_ONCE_INIT;
static int once_count;

static void inc_once(void)
{
    once_count++;
}

static void *tls_worker(void *arg)
{
    pthread_once(&once_ctl, inc_once);
    pthread_setspecific(tls_key, arg);
    return pthread_getspecific(tls_key);
}

static const char *test_pthread_tls(void)
{
    pthread_key_create(&tls_key, NULL);
    int a = 1, b = 2;
    pthread_t t1, t2;
    pthread_create(&t1, NULL, tls_worker, &a);
    pthread_create(&t2, NULL, tls_worker, &b);
    void *r1 = NULL, *r2 = NULL;
    pthread_join(t1, &r1);
    pthread_join(t2, &r2);
    pthread_key_delete(tls_key);
    mu_assert("tls 1", r1 == &a);
    mu_assert("tls 2", r2 == &b);
    mu_assert("once", once_count == 1);
    return 0;
}

static const char *test_pthread_mutexattr(void)
{
    pthread_mutexattr_t attr;
    int type = -1;
    mu_assert("attr init", pthread_mutexattr_init(&attr) == 0);
    mu_assert("attr default", pthread_mutexattr_gettype(&attr, &type) == 0 &&
                              type == PTHREAD_MUTEX_NORMAL);
    mu_assert("attr set", pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) == 0);
    mu_assert("attr get", pthread_mutexattr_gettype(&attr, &type) == 0 &&
                             type == PTHREAD_MUTEX_RECURSIVE);
    mu_assert("attr destroy", pthread_mutexattr_destroy(&attr) == 0);
    return 0;
}

static void *trylock_worker(void *arg)
{
    pthread_mutex_t *m = arg;
    int r = pthread_mutex_trylock(m);
    if (r == 0)
        pthread_mutex_unlock(m);
    return (void *)(long)r;
}

static const char *test_pthread_mutex_recursive(void)
{
    pthread_mutex_t m;
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&m, &attr);
    pthread_mutexattr_destroy(&attr);

    mu_assert("lock1", pthread_mutex_lock(&m) == 0);
    mu_assert("lock2", pthread_mutex_lock(&m) == 0);

    pthread_t t;
    pthread_create(&t, NULL, trylock_worker, &m);
    void *r = NULL;
    pthread_join(t, &r);
    mu_assert("other busy", (long)r == EBUSY);

    mu_assert("self trylock", pthread_mutex_trylock(&m) == 0);

    mu_assert("unlock1", pthread_mutex_unlock(&m) == 0);
    mu_assert("unlock2", pthread_mutex_unlock(&m) == 0);
    mu_assert("unlock3", pthread_mutex_unlock(&m) == 0);

    pthread_create(&t, NULL, trylock_worker, &m);
    pthread_join(t, &r);
    mu_assert("other success", (long)r == 0);

    pthread_mutex_destroy(&m);
    return 0;
}

static const char *test_pthread_attr_basic(void)
{
    pthread_attr_t attr;
    size_t sz = 0;
    int st = -1;
    mu_assert("init", pthread_attr_init(&attr) == 0);
    mu_assert("get default stack", pthread_attr_getstacksize(&attr, &sz) == 0 && sz == 0);
    mu_assert("set stack", pthread_attr_setstacksize(&attr, 65536) == 0);
    mu_assert("get stack", pthread_attr_getstacksize(&attr, &sz) == 0 && sz == 65536);
    mu_assert("get detach default", pthread_attr_getdetachstate(&attr, &st) == 0 && st == PTHREAD_CREATE_JOINABLE);
    mu_assert("set detach", pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) == 0);
    mu_assert("get detach", pthread_attr_getdetachstate(&attr, &st) == 0 && st == PTHREAD_CREATE_DETACHED);
    mu_assert("destroy", pthread_attr_destroy(&attr) == 0);
    return 0;
}

static pthread_rwlock_t rwlock;
static int rwval;

static void *rwreader(void *arg)
{
    (void)arg;
    pthread_rwlock_rdlock(&rwlock);
    int v = rwval;
    pthread_rwlock_unlock(&rwlock);
    return (void *)(long)v;
}

static void *rwwriter(void *arg)
{
    int inc = *(int *)arg;
    pthread_rwlock_wrlock(&rwlock);
    rwval += inc;
    pthread_rwlock_unlock(&rwlock);
    return NULL;
}

static const char *test_pthread_rwlock(void)
{
    pthread_rwlock_init(&rwlock, NULL);
    rwval = 0;
    int inc = 5;
    pthread_t tw, tr;
    pthread_create(&tw, NULL, rwwriter, &inc);
    pthread_create(&tr, NULL, rwreader, NULL);
    void *r = NULL;
    pthread_join(tw, NULL);
    pthread_join(tr, &r);
    pthread_rwlock_destroy(&rwlock);
    mu_assert("rw value", rwval == 5);
    mu_assert("rw read", (long)r == 5);
    return 0;
}

static sem_t sem;

static void *sem_worker(void *arg)
{
    sem_wait(&sem);
    return arg;
}

static const char *test_semaphore_basic(void)
{
    sem_init(&sem, 0, 0);
    pthread_t t;
    pthread_create(&t, NULL, sem_worker, (void *)123);
    usleep(100000);
    sem_post(&sem);
    void *r = NULL;
    pthread_join(t, &r);
    sem_destroy(&sem);
    mu_assert("sem result", r == (void *)123);
    return 0;
}

static const char *test_semaphore_trywait(void)
{
    sem_t s;
    sem_init(&s, 0, 1);
    mu_assert("trywait first", sem_trywait(&s) == 0);
    mu_assert("trywait empty", sem_trywait(&s) == EAGAIN);
    sem_destroy(&s);
    return 0;
}

static pthread_barrier_t barrier;
static int barrier_step[3];

static void *barrier_worker(void *arg)
{
    int idx = *(int *)arg;
    barrier_step[idx] = 1;
    pthread_barrier_wait(&barrier);
    barrier_step[idx] = 2;
    return NULL;
}

static const char *test_pthread_barrier(void)
{
    pthread_t t1, t2;
    int i1 = 0, i2 = 1;
    memset(barrier_step, 0, sizeof(barrier_step));
    pthread_barrier_init(&barrier, NULL, 3);
    pthread_create(&t1, NULL, barrier_worker, &i1);
    pthread_create(&t2, NULL, barrier_worker, &i2);
    barrier_step[2] = 1;
    pthread_barrier_wait(&barrier);
    barrier_step[2] = 2;
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_barrier_destroy(&barrier);
    mu_assert("barrier phase1", barrier_step[0] == 1 && barrier_step[1] == 1 && barrier_step[2] == 2);
    mu_assert("barrier phase2", barrier_step[0] == 2 && barrier_step[1] == 2);
    return 0;
}

static pthread_spinlock_t spin;
static int spin_counter;

static void *spin_worker(void *arg)
{
    (void)arg;
    for (int i = 0; i < 1000; ++i) {
        pthread_spin_lock(&spin);
        spin_counter++;
        pthread_spin_unlock(&spin);
    }
    return NULL;
}

static const char *test_pthread_spinlock(void)
{
    pthread_t t1, t2;
    pthread_spin_init(&spin, 0);
    spin_counter = 0;
    pthread_create(&t1, NULL, spin_worker, NULL);
    pthread_create(&t2, NULL, spin_worker, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    mu_assert("spin count", spin_counter == 2000);
    mu_assert("trylock avail", pthread_spin_trylock(&spin) == 0);
    mu_assert("trylock busy", pthread_spin_trylock(&spin) == EBUSY);
    pthread_spin_unlock(&spin);
    pthread_spin_destroy(&spin);
    return 0;
}

static pthread_mutex_t cond_mutex;
static pthread_cond_t cond_var;
static int cond_step;
static int cond_woken[2];

struct cond_arg {
    int step;
    int idx;
};

static void *cond_worker(void *arg)
{
    struct cond_arg *a = arg;
    pthread_mutex_lock(&cond_mutex);
    while (cond_step < a->step)
        pthread_cond_wait(&cond_var, &cond_mutex);
    cond_woken[a->idx] = 1;
    pthread_mutex_unlock(&cond_mutex);
    return NULL;
}

static const char *test_pthread_cond_signal(void)
{
    pthread_t t1, t2;
    struct cond_arg a1 = {1, 0};
    struct cond_arg a2 = {2, 1};
    pthread_mutex_init(&cond_mutex, NULL);
    pthread_cond_init(&cond_var, NULL);
    cond_step = 0;
    cond_woken[0] = cond_woken[1] = 0;
    pthread_create(&t1, NULL, cond_worker, &a1);
    pthread_create(&t2, NULL, cond_worker, &a2);
    usleep(100000);
    pthread_mutex_lock(&cond_mutex);
    cond_step = 1;
    pthread_cond_signal(&cond_var);
    pthread_mutex_unlock(&cond_mutex);
    usleep(100000);
    int first_woken = cond_woken[0] + cond_woken[1];
    pthread_mutex_lock(&cond_mutex);
    cond_step = 2;
    pthread_cond_signal(&cond_var);
    pthread_mutex_unlock(&cond_mutex);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_cond_destroy(&cond_var);
    pthread_mutex_destroy(&cond_mutex);
    mu_assert("first signal woke one", first_woken == 1);
    mu_assert("both signaled", cond_woken[0] == 1 && cond_woken[1] == 1);
    return 0;
}

static const char *test_pthread_cond_broadcast(void)
{
    pthread_t t1, t2;
    struct cond_arg a1 = {1, 0};
    struct cond_arg a2 = {1, 1};
    pthread_mutex_init(&cond_mutex, NULL);
    pthread_cond_init(&cond_var, NULL);
    cond_step = 0;
    cond_woken[0] = cond_woken[1] = 0;
    pthread_create(&t1, NULL, cond_worker, &a1);
    pthread_create(&t2, NULL, cond_worker, &a2);
    usleep(100000);
    pthread_mutex_lock(&cond_mutex);
    cond_step = 1;
    pthread_cond_broadcast(&cond_var);
    pthread_mutex_unlock(&cond_mutex);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_cond_destroy(&cond_var);
    pthread_mutex_destroy(&cond_mutex);
    mu_assert("broadcast woke all", cond_woken[0] == 1 && cond_woken[1] == 1);
    return 0;
}

static void *delayed_write(void *arg)
{
    int fd = *(int *)arg;
    usleep(100000);
    write(fd, "z", 1);
    return NULL;
}

static const char *test_select_pipe(void)
{
    int p[2];
    mu_assert("pipe", pipe(p) == 0);

    pthread_t t;
    pthread_create(&t, NULL, delayed_write, &p[1]);

    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(p[0], &rfds);
    struct timeval tv = {2, 0};

    int r = select(p[0] + 1, &rfds, NULL, NULL, &tv);
    pthread_join(t, NULL);
    mu_assert("select ret", r == 1);
    mu_assert("fd set", FD_ISSET(p[0], &rfds));

    char c;
    mu_assert("read", read(p[0], &c, 1) == 1 && c == 'z');

    close(p[0]);
    close(p[1]);
    return 0;
}

static const char *test_poll_pipe(void)
{
    int p[2];
    mu_assert("pipe", pipe(p) == 0);

    pthread_t t;
    pthread_create(&t, NULL, delayed_write, &p[1]);

    struct pollfd fds[1];
    fds[0].fd = p[0];
    fds[0].events = POLLIN;

    int r = poll(fds, 1, 2000);
    pthread_join(t, NULL);
    mu_assert("poll ret", r == 1);
    mu_assert("poll event", fds[0].revents & POLLIN);

    char c;
    mu_assert("read", read(p[0], &c, 1) == 1 && c == 'z');

    close(p[0]);
    close(p[1]);
    return 0;
}

static const char *test_sleep_functions(void)
{
    time_t t1 = time(NULL);
    unsigned r = sleep(1);
    time_t t2 = time(NULL);
    mu_assert("sleep returned", r == 0);
    mu_assert("sleep delay", t2 - t1 >= 1 && t2 - t1 <= 3);

    t1 = time(NULL);
    mu_assert("usleep failed", usleep(500000) == 0);
    mu_assert("usleep failed2", usleep(500000) == 0);
    t2 = time(NULL);
    mu_assert("usleep delay", t2 - t1 >= 1 && t2 - t1 <= 3);

    struct timespec ts = {1, 0};
    t1 = time(NULL);
    mu_assert("nanosleep failed", nanosleep(&ts, NULL) == 0);
    t2 = time(NULL);
    mu_assert("nanosleep delay", t2 - t1 >= 1 && t2 - t1 <= 3);

    return 0;
}

static const char *test_clock_nanosleep_basic(void)
{
    struct timespec ts, start, end;

    ts.tv_sec = 1;
    ts.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &start);
    mu_assert("clock_nanosleep rel",
             clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL) == 0);
    clock_gettime(CLOCK_MONOTONIC, &end);
    mu_assert("rel delay", end.tv_sec - start.tv_sec >= 1 &&
                             end.tv_sec - start.tv_sec <= 3);

    clock_gettime(CLOCK_MONOTONIC, &start);
    ts = start;
    ts.tv_sec += 1;
    mu_assert("clock_nanosleep abs",
             clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts, NULL) == 0);
    clock_gettime(CLOCK_MONOTONIC, &end);
    mu_assert("abs delay", end.tv_sec - start.tv_sec >= 1 &&
                            end.tv_sec - start.tv_sec <= 3);

    return 0;
}

static const char *test_sched_yield_basic(void)
{
    mu_assert("sched_yield", sched_yield() == 0);
    return 0;
}

static const char *test_sched_yield_loop(void)
{
    for (int i = 0; i < 10; ++i)
        mu_assert("sched_yield", sched_yield() == 0);
    return 0;
}

static const char *test_priority_wrappers(void)
{
    int orig = getpriority(PRIO_PROCESS, 0);
    mu_assert("getpriority", orig != -1 || errno == 0);
    mu_assert("setpriority", setpriority(PRIO_PROCESS, 0, orig + 1) == 0);
    mu_assert("verify", getpriority(PRIO_PROCESS, 0) == orig + 1);
    mu_assert("nice", nice(-1) == orig);
    mu_assert("restore", getpriority(PRIO_PROCESS, 0) == orig);
    return 0;
}

static const char *test_sched_get_set_scheduler(void)
{
    struct sched_param sp;
    int pol = sched_getscheduler(0);
    if (pol == -1 && errno == ENOSYS)
        return 0;
    mu_assert("sched_getscheduler", pol >= 0);
    mu_assert("sched_getparam", sched_getparam(0, &sp) == 0);
    mu_assert("sched_setscheduler", sched_setscheduler(0, pol, &sp) == 0);
    mu_assert("verify", sched_getscheduler(0) == pol);
    return 0;
}

static const char *test_timer_basic(void)
{
    timer_t t;
    struct itimerspec its = { {0, 0}, {0, 200000000} };
    mu_assert("timer_create", timer_create(CLOCK_REALTIME, NULL, &t) == 0);
    mu_assert("timer_settime", timer_settime(t, 0, &its, NULL) == 0);
    struct timespec ts = {0, 300000000};
    nanosleep(&ts, NULL);
    struct itimerspec cur;
    mu_assert("timer_gettime", timer_gettime(t, &cur) == 0);
    mu_assert("expired", cur.it_value.tv_sec == 0 && cur.it_value.tv_nsec == 0);
    mu_assert("timer_delete", timer_delete(t) == 0);
    return 0;
}

static const char *test_clock_settime_priv(void)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
        return "gettime";
    int r = clock_settime(CLOCK_MONOTONIC, &ts);
    if (r != 0) {
        if (errno == EPERM || errno == ENOSYS || errno == EINVAL)
            return 0; /* skip when not permitted */
        return "clock_settime";
    }
    struct timespec check;
    mu_assert("verify", clock_gettime(CLOCK_MONOTONIC, &check) == 0);
    mu_assert("compare", check.tv_sec >= ts.tv_sec);
    return 0;
}

static const char *test_getrusage_self(void)
{
    struct rusage r;
    volatile long sink = 0;
    for (long i = 0; i < 1000000; ++i)
        sink += i;
    (void)sink;
    mu_assert("getrusage", getrusage(RUSAGE_SELF, &r) == 0);
    mu_assert("have utime", r.ru_utime.tv_sec > 0 || r.ru_utime.tv_usec > 0);
    return 0;
}

static const char *test_times_self(void)
{
    struct tms t;
    volatile long sink = 0;
    for (long i = 0; i < 1000000; ++i)
        sink += i;
    (void)sink;
    clock_t c = times(&t);
    mu_assert("times", c != (clock_t)-1);
    mu_assert("have ticks", t.tms_utime > 0 || t.tms_stime > 0);
    return 0;
}

static const char *test_getloadavg_basic(void)
{
    double l[3];
    int n = getloadavg(l, 3);
    mu_assert("getloadavg", n >= 1);
    for (int i = 0; i < n; ++i)
        mu_assert("nonnegative", l[i] >= 0.0);
    return 0;
}

static const char *test_timespec_get_basic(void)
{
    struct timespec ts;
    int r = timespec_get(&ts, TIME_UTC);
    mu_assert("timespec_get ret", r == TIME_UTC);
    mu_assert("timespec_get sec", ts.tv_sec > 0);
    return 0;
}

static const char *test_strftime_basic(void)
{
    struct tm tm = {
        .tm_year = 123,
        .tm_mon = 4,
        .tm_mday = 6,
        .tm_hour = 7,
        .tm_min = 8,
        .tm_sec = 9
    };
    char buf[32];
    size_t n = strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
    mu_assert("strftime len", n == strlen("2023-05-06 07:08:09"));
    mu_assert("strftime str", strcmp(buf, "2023-05-06 07:08:09") == 0);
    return 0;
}

static const char *test_strftime_extended(void)
{
    struct tm tm = {
        .tm_year = 123,
        .tm_mon = 4,
        .tm_mday = 6,
        .tm_wday = 6,
        .tm_hour = 7,
        .tm_min = 8,
        .tm_sec = 9
    };
    char buf[64];
    size_t n = strftime(buf, sizeof(buf), "%a %b %d %Y %H:%M:%S %Z %z %w %u", &tm);
    mu_assert("strftime len2", n == strlen("Sat May 06 2023 07:08:09 UTC +0000 6 6"));
    mu_assert("strftime str2", strcmp(buf, "Sat May 06 2023 07:08:09 UTC +0000 6 6") == 0);
    return 0;
}

static const char *test_wcsftime_basic(void)
{
    struct tm tm = {
        .tm_year = 123,
        .tm_mon = 4,
        .tm_mday = 6,
        .tm_hour = 7,
        .tm_min = 8,
        .tm_sec = 9
    };
    wchar_t buf[32];
    size_t n = wcsftime(buf, sizeof(buf)/sizeof(wchar_t), L"%Y-%m-%d %H:%M:%S", &tm);
    mu_assert("wcsftime len", n == wcslen(L"2023-05-06 07:08:09"));
    mu_assert("wcsftime str", wcscmp(buf, L"2023-05-06 07:08:09") == 0);
    return 0;
}

static const char *test_wcsftime_extended(void)
{
    struct tm tm = {
        .tm_year = 123,
        .tm_mon = 4,
        .tm_mday = 6,
        .tm_wday = 6,
        .tm_hour = 7,
        .tm_min = 8,
        .tm_sec = 9
    };
    wchar_t buf[64];
    size_t n = wcsftime(buf, sizeof(buf)/sizeof(wchar_t),
                        L"%a %b %d %Y %H:%M:%S %Z %z %w %u", &tm);
    mu_assert("wcsftime len2", n == wcslen(L"Sat May 06 2023 07:08:09 UTC +0000 6 6"));
    mu_assert("wcsftime str2", wcscmp(buf, L"Sat May 06 2023 07:08:09 UTC +0000 6 6") == 0);
    return 0;
}

static const char *test_strptime_basic(void)
{
    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    char *r = strptime("2023-05-06 07:08:09", "%Y-%m-%d %H:%M:%S", &tm);
    mu_assert("strptime ret", r && *r == '\0');
    mu_assert("tm year", tm.tm_year == 123);
    mu_assert("tm mon", tm.tm_mon == 4);
    mu_assert("tm mday", tm.tm_mday == 6);
    mu_assert("tm hour", tm.tm_hour == 7);
    mu_assert("tm min", tm.tm_min == 8);
    mu_assert("tm sec", tm.tm_sec == 9);
    return 0;
}

static const char *test_time_conversions(void)
{
    time_t t = 1700000000;
    struct tm *gm = gmtime(&t);
    mu_assert("gm year", gm->tm_year == 123);
    mu_assert("gm mon", gm->tm_mon == 10);
    mu_assert("gm mday", gm->tm_mday == 14);
    mu_assert("gm hour", gm->tm_hour == 22);
    mu_assert("gm min", gm->tm_min == 13);
    mu_assert("gm sec", gm->tm_sec == 20);
    mu_assert("gm wday", gm->tm_wday == 2);

    struct tm *loc = localtime(&t);
    mu_assert("localtime", loc->tm_yday == gm->tm_yday && loc->tm_mon == gm->tm_mon);

    struct tm tmp = *gm;
    time_t r = mktime(&tmp);
    mu_assert("mktime", r == 1700000000);

    tmp = *gm;
    r = timegm(&tmp);
    mu_assert("timegm", r == 1700000000);

    char *s = ctime(&t);
    mu_assert("ctime", strcmp(s, "Tue Nov 14 22:13:20 2023\n") == 0);
    char *a = asctime(gm);
    mu_assert("asctime", strcmp(a, "Tue Nov 14 22:13:20 2023\n") == 0);
    char buf[32];
    mu_assert("asctime_r",
              strcmp(asctime_r(gm, buf), "Tue Nov 14 22:13:20 2023\n") == 0);
    return 0;
}

static const char *test_time_r_conversions(void)
{
    time_t t = 1700000000;
    struct tm tm1;
    struct tm tm2;
    tzset();
    mu_assert("gmtime_r", gmtime_r(&t, &tm1) != NULL);
    mu_assert("localtime_r", localtime_r(&t, &tm2) != NULL);
    mu_assert("match", tm1.tm_yday == tm2.tm_yday && tm1.tm_mon == tm2.tm_mon);
    return 0;
}

static const char *test_timegm_known_values(void)
{
    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    tm.tm_year = 100; /* 2000 */
    tm.tm_mon = 0;
    tm.tm_mday = 1;
    time_t r = timegm(&tm);
    mu_assert("timegm 2000", r == 946684800);

    memset(&tm, 0, sizeof(tm));
    tm.tm_year = 124; /* 2024 */
    tm.tm_mon = 1;    /* Feb */
    tm.tm_mday = 29;
    tm.tm_hour = 12;
    tm.tm_min = 34;
    tm.tm_sec = 56;
    r = timegm(&tm);
    mu_assert("timegm leap", r == 1709210096);

    memset(&tm, 0, sizeof(tm));
    tm.tm_year = 138; /* 2038 */
    tm.tm_mon = 0;
    tm.tm_mday = 19;
    tm.tm_hour = 3;
    tm.tm_min = 14;
    tm.tm_sec = 7;
    r = timegm(&tm);
    mu_assert("timegm 2038", r == 2147483647);

    return 0;
}

static const char *test_difftime_basic(void)
{
    time_t a = 10;
    time_t b = 42;
    double d = difftime(b, a);
    mu_assert("difftime pos", fabs(d - 32.0) < 1e-9);
    d = difftime(a, b);
    mu_assert("difftime neg", fabs(d + 32.0) < 1e-9);
    return 0;
}

static const char *test_tz_positive(void)
{
    setenv("TZ", "UTC+2", 1);
    tzset();
    time_t t = 0;
    struct tm tm;
    mu_assert("pos localtime", localtime_r(&t, &tm) != NULL);
    mu_assert("hour plus", tm.tm_hour == 2);
    unsetenv("TZ");
    tzset();
    return 0;
}

static const char *test_tz_negative(void)
{
    setenv("TZ", "UTC-3", 1);
    tzset();
    time_t t = 4 * 3600;
    struct tm tm;
    mu_assert("neg localtime", localtime_r(&t, &tm) != NULL);
    mu_assert("hour minus", tm.tm_hour == 1);
    unsetenv("TZ");
    tzset();
    return 0;
}

static const char *test_tz_mktime_roundtrip(void)
{
    setenv("TZ", "UTC+1", 1);
    tzset();
    time_t t = 1700000000;
    struct tm tm;
    localtime_r(&t, &tm);
    time_t r = mktime(&tm);
    unsetenv("TZ");
    tzset();
    mu_assert("mktime round", r == t);
    return 0;
}

static const char *test_tz_ctime(void)
{
    setenv("TZ", "UTC+1", 1);
    tzset();
    time_t t = 1700000000;
    char *s = ctime(&t);
    unsetenv("TZ");
    tzset();
    mu_assert("ctime offset", strstr(s, "23:13:20") != NULL);
    return 0;
}

static const char *test_asctime_r_threadsafe(void)
{
    time_t t1 = 1700000000;
    time_t t2 = t1 + 86400;
    struct tm tm1, tm2;
    gmtime_r(&t1, &tm1);
    gmtime_r(&t2, &tm2);
    struct asctime_arg a1 = { tm1, "Tue Nov 14 22:13:20 2023\n" };
    struct asctime_arg a2 = { tm2, "Wed Nov 15 22:13:20 2023\n" };
    pthread_t th1, th2;
    pthread_create(&th1, NULL, asctime_r_worker, &a1);
    pthread_create(&th2, NULL, asctime_r_worker, &a2);
    void *r1 = (void *)1;
    void *r2 = (void *)1;
    pthread_join(th1, &r1);
    pthread_join(th2, &r2);
    mu_assert("asctime_r thread1", r1 == NULL);
    mu_assert("asctime_r thread2", r2 == NULL);
    return 0;
}

static const char *test_environment(void)
{
    env_init(NULL);
    mu_assert("empty env", getenv("FOO") == NULL);

    int r = setenv("FOO", "BAR", 0);
    mu_assert("setenv new", r == 0);
    char *v = getenv("FOO");
    mu_assert("getenv new", v && strcmp(v, "BAR") == 0);

    r = setenv("FOO", "BAZ", 0);
    v = getenv("FOO");
    mu_assert("no overwrite", v && strcmp(v, "BAR") == 0);

    r = setenv("FOO", "BAZ", 1);
    mu_assert("overwrite", r == 0);
    v = getenv("FOO");
    mu_assert("getenv overwrite", v && strcmp(v, "BAZ") == 0);

    unsetenv("FOO");
    mu_assert("unsetenv", getenv("FOO") == NULL);

    return 0;
}

static const char *test_clearenv_fn(void)
{
    env_init(NULL);
    setenv("A", "1", 1);
    setenv("B", "2", 1);
    mu_assert("set before", getenv("A") && getenv("B"));

    mu_assert("clearenv", clearenv() == 0);
    mu_assert("cleared", getenv("A") == NULL && getenv("B") == NULL);
    mu_assert("environ null", environ && environ[0] == NULL);

    setenv("C", "3", 1);
    mu_assert("after clear", getenv("C") && strcmp(getenv("C"), "3") == 0);

    clearenv();
    return 0;
}

static const char *test_env_init_clearenv(void)
{
    extern char **__environ;
    int count = 0;
    while (__environ && __environ[count])
        count++;
    char **copy = malloc(sizeof(char *) * (count + 1));
    for (int i = 0; i < count; ++i)
        copy[i] = __environ[i];
    copy[count] = NULL;

    env_init(copy);
    mu_assert("clearenv", clearenv() == 0);
    mu_assert("same pointer", environ == copy);
    mu_assert("first null", copy[0] == NULL);
    env_init(NULL);
    free(copy);
    return 0;
}

static const char *test_locale_from_env(void)
{
    env_init(NULL);
    unsetenv("LC_ALL");
    unsetenv("LANG");

    mu_assert("default C", strcmp(setlocale(LC_ALL, ""), "C") == 0);

    setenv("LANG", "C.UTF-8", 1);
    char *r = setlocale(LC_ALL, "");
#ifdef __linux__
    mu_assert("unsupported locale", r == NULL);
#else
    mu_assert("system locale", r && strcmp(r, "C.UTF-8") == 0);
#endif
    unsetenv("LANG");

    setenv("LC_ALL", "POSIX", 1);
    mu_assert("LC_ALL", strcmp(setlocale(LC_ALL, ""), "POSIX") == 0);
    unsetenv("LC_ALL");

    return 0;
}

static const char *test_locale_objects(void)
{
    locale_t loc = newlocale(LC_ALL, "C", NULL);
    mu_assert("newlocale", loc != NULL);
    locale_t old = uselocale(loc);
    struct lconv *lc = localeconv();
    mu_assert("decimal_point", strcmp(lc->decimal_point, ".") == 0);
    mu_assert("thousands_sep", strcmp(lc->thousands_sep, "") == 0);
    uselocale(old);
    freelocale(loc);
    return 0;
}

static const char *test_gethostname_fn(void)
{
    char buf[256];
    mu_assert("gethostname", gethostname(buf, sizeof(buf)) == 0);
    mu_assert("non-empty", buf[0] != '\0');
    return 0;
}

static const char *test_uname_fn(void)
{
    struct utsname u;
    mu_assert("uname", uname(&u) == 0);
    mu_assert("sysname", u.sysname[0] != '\0');
    mu_assert("release", u.release[0] != '\0');
    return 0;
}

static const char *test_confstr_path(void)
{
#ifdef _CS_PATH
    char buf[256];
    errno = 0;
    size_t n = confstr(_CS_PATH, buf, sizeof(buf));
#if defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__OpenBSD__) || defined(__DragonFly__)
    mu_assert("confstr path", n > 0 && n < sizeof(buf));
    mu_assert("non-empty", buf[0] != '\0');
#else
    mu_assert("confstr unsupported", n == 0);
    mu_assert("errno EINVAL", errno == EINVAL);
#endif
#else
    errno = 0;
    size_t n = confstr(0, NULL, 0);
    mu_assert("confstr absent", n == 0 && errno == EINVAL);
#endif
    return 0;
}

static const char *test_progname_setget(void)
{
    setprogname("prog");
    const char *p = getprogname();
    mu_assert("initial name", p && strcmp(p, "prog") == 0);

    setprogname("/usr/bin/testprog");
    p = getprogname();
    mu_assert("basename", p && strcmp(p, "testprog") == 0);

    return 0;
}

static const char *test_error_reporting(void)
{
    errno = ENOENT;
    char *msg1 = strerror(errno);
    mu_assert("strerror", msg1 && *msg1 != '\0');
    perror("test");
    vlibc_init();
    const char *msg2 = strerror(ENOENT);
    mu_assert("strerror", strcmp(msg2, "No such file or directory") == 0);

    int p[2];
    mu_assert("pipe", pipe(p) == 0);
    int old = dup(2);
    mu_assert("dup", old >= 0);
    dup2(p[1], 2);
    close(p[1]);
    errno = ENOENT;
    perror("test");
    dup2(old, 2);
    close(old);
    char buf[64] = {0};
    ssize_t n = read(p[0], buf, sizeof(buf) - 1);
    close(p[0]);
    mu_assert("perror read", n > 0);
    const char *exp = "test: No such file or directory\n";
    mu_assert("perror output", (size_t)n == strlen(exp) && memcmp(buf, exp, n) == 0);

    pthread_t t1, t2;
    int e1 = ENOENT;
    int e2 = 9999;
    pthread_create(&t1, NULL, strerror_r_worker, &e1);
    pthread_create(&t2, NULL, strerror_r_worker, &e2);
    void *r1 = (void *)1;
    void *r2 = (void *)1;
    pthread_join(t1, &r1);
    pthread_join(t2, &r2);
    mu_assert("strerror_r thread1", r1 == NULL);
    mu_assert("strerror_r thread2", r2 == NULL);

    mu_assert("EPROTO", strcmp(strerror(EPROTO), "Protocol error") == 0);
    mu_assert("EOVERFLOW", strcmp(strerror(EOVERFLOW),
                "Value too large to be stored in data type") == 0);
    mu_assert("EHOSTDOWN", strcmp(strerror(EHOSTDOWN), "Host is down") == 0);
    mu_assert("EOWNERDEAD", strcmp(strerror(EOWNERDEAD),
                "Previous owner died") == 0);

    return 0;
}

static int call_vwarn(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vwarn(fmt, ap);
    va_end(ap);
    return 0;
}

static const char *test_warn_functions(void)
{
    int p[2];
    mu_assert("pipe", pipe(p) == 0);
    int old = dup(2);
    mu_assert("dup", old >= 0);
    dup2(p[1], 2);
    close(p[1]);
    errno = ENOENT;
    warn("missing %s", "file");
    dup2(old, 2);
    close(old);
    char buf[80] = {0};
    ssize_t n = read(p[0], buf, sizeof(buf) - 1);
    close(p[0]);
    mu_assert("warn output", n > 0 && strcmp(buf, "missing file: No such file or directory\n") == 0);

    mu_assert("pipe", pipe(p) == 0);
    old = dup(2);
    mu_assert("dup", old >= 0);
    dup2(p[1], 2);
    close(p[1]);
    warnx("fatal %d", 5);
    dup2(old, 2);
    close(old);
    memset(buf, 0, sizeof(buf));
    n = read(p[0], buf, sizeof(buf) - 1);
    close(p[0]);
    mu_assert("warnx output", n > 0 && strcmp(buf, "fatal 5\n") == 0);

    mu_assert("pipe", pipe(p) == 0);
    old = dup(2);
    dup2(p[1], 2);
    close(p[1]);
    errno = ENOENT;
    call_vwarn("try %s", "again");
    dup2(old, 2);
    close(old);
    memset(buf, 0, sizeof(buf));
    n = read(p[0], buf, sizeof(buf) - 1);
    close(p[0]);
    mu_assert("vwarn output", n > 0 && strcmp(buf, "try again: No such file or directory\n") == 0);

    return 0;
}

static const char *test_err_functions(void)
{
    int p[2];
    mu_assert("pipe", pipe(p) == 0);
    pid_t pid = fork();
    mu_assert("fork", pid >= 0);
    if (pid == 0) {
        dup2(p[1], 2);
        close(p[0]);
        errno = ENOENT;
        err(7, "open %s", "file");
    }
    close(p[1]);
    char buf[80] = {0};
    ssize_t n = read(p[0], buf, sizeof(buf) - 1);
    close(p[0]);
    int status = 0;
    waitpid(pid, &status, 0);
    mu_assert("err exit", WIFEXITED(status) && WEXITSTATUS(status) == 7);
    mu_assert("err output", n > 0 && strcmp(buf, "open file: No such file or directory\n") == 0);

    mu_assert("pipe", pipe(p) == 0);
    pid = fork();
    mu_assert("fork", pid >= 0);
    if (pid == 0) {
        dup2(p[1], 2);
        close(p[0]);
        errx(3, "fatal %s", "bug");
    }
    close(p[1]);
    memset(buf, 0, sizeof(buf));
    n = read(p[0], buf, sizeof(buf) - 1);
    close(p[0]);
    status = 0;
    waitpid(pid, &status, 0);
    mu_assert("errx exit", WIFEXITED(status) && WEXITSTATUS(status) == 3);
    mu_assert("errx output", n > 0 && strcmp(buf, "fatal bug\n") == 0);

    return 0;
}

static const char *test_strsignal_names(void)
{
    mu_assert("SIGHUP", strcmp(strsignal(SIGHUP), "Hangup") == 0);
    mu_assert("SIGINT", strcmp(strsignal(SIGINT), "Interrupt") == 0);
    mu_assert("unknown", strcmp(strsignal(9999), "Unknown signal") == 0);
    return 0;
}

static const char *test_pid_functions(void)
{
    pid_t pid = getpid();
    pid_t ppid = getppid();
    mu_assert("getpid", pid > 0);
    mu_assert("getppid", ppid >= 0);

    return 0;
}

static const char *test_process_group_wrappers(void)
{
    pid_t original = getpgrp();
    mu_assert("orig", original > 0);

    pid_t pid = fork();
    mu_assert("fork", pid >= 0);
    if (pid == 0) {
        if (setpgrp() != 0)
            _exit(1);
        pid_t pg = getpgrp();
        if (pg != getpid())
            _exit(2);
        _exit(0);
    }
    int status = 0;
    waitpid(pid, &status, 0);
    mu_assert("child", WIFEXITED(status) && WEXITSTATUS(status) == 0);
    mu_assert("unchanged", getpgrp() == original);

    return 0;
}

static const char *test_system_fn(void)
{
    int r = system("true");
    mu_assert("system true", r == 0);
    r = system("exit 7");
    mu_assert("system exit code", (r >> 8) == 7);
    return 0;
}

static const char *test_system_interrupted(void)
{
    struct sigaction sa;
    sa.sa_handler = handle_usr1;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGUSR1, &sa, NULL);

    pthread_t t;
    int sig = SIGUSR1;
    pthread_create(&t, NULL, send_signal, &sig);

    int r = system("sleep 1");
    pthread_join(t, NULL);

    mu_assert("system interrupted", r == 0);
    return 0;
}

static const char *test_execv_fn(void)
{
    extern char **__environ;
    env_init(__environ);
    pid_t pid = fork();
    mu_assert("fork", pid >= 0);
    if (pid == 0) {
        char *argv[] = {"/bin/echo", "v", NULL};
        execv("/bin/echo", argv);
        _exit(127);
    }
    int status = 0;
    waitpid(pid, &status, 0);
    mu_assert("execv status", WIFEXITED(status) && WEXITSTATUS(status) == 0);
    return 0;
}

static const char *test_execl_fn(void)
{
    extern char **__environ;
    env_init(__environ);
    pid_t pid = fork();
    mu_assert("fork", pid >= 0);
    if (pid == 0) {
        execl("/bin/echo", "l", "1", NULL);
        _exit(127);
    }
    int status = 0;
    waitpid(pid, &status, 0);
    mu_assert("execl status", WIFEXITED(status) && WEXITSTATUS(status) == 0);
    return 0;
}

static const char *test_execlp_fn(void)
{
    extern char **__environ;
    env_init(__environ);
    pid_t pid = fork();
    mu_assert("fork", pid >= 0);
    if (pid == 0) {
        execlp("echo", "lp", NULL);
        _exit(127);
    }
    int status = 0;
    waitpid(pid, &status, 0);
    mu_assert("execlp status", WIFEXITED(status) && WEXITSTATUS(status) == 0);
    return 0;
}

static const char *test_execle_fn(void)
{
    extern char **__environ;
    env_init(__environ);
    pid_t pid = fork();
    mu_assert("fork", pid >= 0);
    if (pid == 0) {
        char *custom[] = {"FOO=BAR", NULL};
        execle("/bin/echo", "le", NULL, custom);
        _exit(127);
    }
    int status = 0;
    waitpid(pid, &status, 0);
    mu_assert("execle status", WIFEXITED(status) && WEXITSTATUS(status) == 0);
    return 0;
}

static const char *test_execvp_fn(void)
{
    extern char **__environ;
    env_init(__environ);
    pid_t pid = fork();
    mu_assert("fork", pid >= 0);
    if (pid == 0) {
        char *argv[] = {"echo", "vp", NULL};
        execvp("echo", argv);
        _exit(127);
    }
    int status = 0;
    waitpid(pid, &status, 0);
    mu_assert("execvp status", WIFEXITED(status) && WEXITSTATUS(status) == 0);
    return 0;
}

static const char *test_fexecve_fn(void)
{
    extern char **__environ;
    env_init(__environ);
    int fd = open("/bin/sh", O_RDONLY);
    mu_assert("open", fd >= 0);
    pid_t pid = fork();
    mu_assert("fork", pid >= 0);
    if (pid == 0) {
        char *argv[] = {"sh", "-c", "exit 7", NULL};
        fexecve(fd, argv, __environ);
        _exit(127);
    }
    close(fd);
    int status = 0;
    waitpid(pid, &status, 0);
    mu_assert("fexecve status", WIFEXITED(status) && WEXITSTATUS(status) == 7);
    return 0;
}

static const char *test_posix_spawn_fn(void)
{
    extern char **__environ;
    env_init(__environ);
    pid_t pid;
    char *argv[] = {"/bin/echo", "spawn", NULL};
    int r = posix_spawn(&pid, "/bin/echo", NULL, NULL, argv, __environ);
    mu_assert("posix_spawn", r == 0);
    int status = 0;
    waitpid(pid, &status, 0);
    mu_assert("spawn status", WIFEXITED(status) && WEXITSTATUS(status) == 0);
    return 0;
}

static const char *test_posix_spawn_actions(void)
{
    extern char **__environ;
    env_init(__environ);
    const char *in = "/tmp/pspawn_in.txt";
    const char *out = "/tmp/pspawn_out.txt";
    int fd = open(in, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    write(fd, "abc", 3);
    close(fd);

    posix_spawn_file_actions_t fa;
    posix_spawn_file_actions_init(&fa);
    posix_spawn_file_actions_addopen(&fa, 0, in, O_RDONLY, 0);
    posix_spawn_file_actions_addopen(&fa, 4, out,
                                    O_WRONLY | O_CREAT | O_TRUNC, 0600);
    posix_spawn_file_actions_adddup2(&fa, 4, 1);
    posix_spawn_file_actions_addclose(&fa, 4);

    char *argv[] = {"/bin/cat", NULL};
    pid_t pid;
    int r = posix_spawn(&pid, "/bin/cat", &fa, NULL, argv, __environ);
    posix_spawn_file_actions_destroy(&fa);
    mu_assert("spawn actions", r == 0);
    int status = 0;
    waitpid(pid, &status, 0);
    mu_assert("cat status", WIFEXITED(status) && WEXITSTATUS(status) == 0);
    fd = open(out, O_RDONLY);
    char buf[4] = {0};
    read(fd, buf, 3);
    close(fd);
    unlink(in);
    unlink(out);
    mu_assert("actions content", strcmp(buf, "abc") == 0);
    return 0;
}

static const char *test_posix_spawn_sigmask(void)
{
    extern char **__environ;
    env_init(__environ);
    const char *out = "/tmp/pspawn_mask.txt";
    posix_spawn_file_actions_t fa;
    posix_spawn_file_actions_init(&fa);
    posix_spawn_file_actions_addopen(&fa, 1, out,
                                    O_WRONLY | O_CREAT | O_TRUNC, 0600);

    posix_spawnattr_t at;
    posix_spawnattr_init(&at);
    sigset_t m;
    sigemptyset(&m);
    sigaddset(&m, SIGUSR1);
    posix_spawnattr_setflags(&at, POSIX_SPAWN_SETSIGMASK);
    posix_spawnattr_setsigmask(&at, &m);

    char *argv[] = {"/bin/sh", "-c", "kill -USR1 $$; echo hi", NULL};
    pid_t pid;
    int r = posix_spawn(&pid, "/bin/sh", &fa, &at, argv, __environ);
    posix_spawn_file_actions_destroy(&fa);
    mu_assert("spawn sigmask", r == 0);
    int status = 0;
    waitpid(pid, &status, 0);
    mu_assert("sigmask status", WIFEXITED(status) && WEXITSTATUS(status) == 0);
    int fd = open(out, O_RDONLY);
    char buf[4] = {0};
    read(fd, buf, 3);
    close(fd);
    unlink(out);
    mu_assert("sigmask content", strcmp(buf, "hi\n") == 0);
    return 0;
}

static const char *test_posix_spawn_pgroup(void)
{
    extern char **__environ;
    env_init(__environ);
    const char *out = "/tmp/pspawn_pgid.txt";

    posix_spawn_file_actions_t fa;
    posix_spawn_file_actions_init(&fa);
    posix_spawn_file_actions_addopen(&fa, 1, out,
                                    O_WRONLY | O_CREAT | O_TRUNC, 0600);

    posix_spawnattr_t at;
    posix_spawnattr_init(&at);
    posix_spawnattr_setflags(&at, POSIX_SPAWN_SETPGROUP);
    posix_spawnattr_setpgroup(&at, 0);
    pid_t tmp;
    posix_spawnattr_getpgroup(&at, &tmp);
    mu_assert("getpgroup", tmp == 0);

    char *argv[] = {"/bin/sh", "-c", "ps -o pgid= -p $$", NULL};
    pid_t pid;
    int r = posix_spawn(&pid, "/bin/sh", &fa, &at, argv, __environ);
    posix_spawn_file_actions_destroy(&fa);
    mu_assert("spawn pgroup", r == 0);
    int status = 0;
    waitpid(pid, &status, 0);
    mu_assert("pgroup status", WIFEXITED(status) && WEXITSTATUS(status) == 0);
    int fd = open(out, O_RDONLY);
    char buf[32] = {0};
    int n = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    unlink(out);
    if (n > 0)
        buf[n] = '\0';
    pid_t pgid = (pid_t)atoi(buf);
    mu_assert("pgid matches pid", pgid == pid);
    return 0;
}

static const char *test_popen_fn(void)
{
    FILE *f = popen("echo popen", "r");
    mu_assert("popen", f != NULL);
    char buf[32] = {0};
    size_t n = fread(buf, 1, sizeof(buf) - 1, f);
    pclose(f);
    mu_assert("popen read", n > 0);
    mu_assert("popen content", strncmp(buf, "popen", 5) == 0);
    return 0;
}

static const char *test_rand_fn(void)
{
    srand(1);
    mu_assert("rand 1", rand() == 16838);
    mu_assert("rand 2", rand() == 5758);
    mu_assert("rand 3", rand() == 10113);
    return 0;
}

static const char *test_rand48_fn(void)
{
    srand48(1L);
    mu_assert("lrand48 1", lrand48() == 89400484);
    mu_assert("lrand48 2", lrand48() == 976015093);
    mu_assert("lrand48 3", lrand48() == 1792756325);
    srand48(1L);
    double d = drand48();
    mu_assert("drand48 1", fabs(d - 0.041630344771878214) < 1e-12);
    d = drand48();
    mu_assert("drand48 2", fabs(d - 0.45449244472862915) < 1e-12);
    unsigned short seed[3] = {0x330e, 0xabcd, 0x1234};
    mu_assert("nrand48 1", nrand48(seed) == 851401618);
    mu_assert("nrand48 2", nrand48(seed) == 1804928587);
    mu_assert("nrand48 3", nrand48(seed) == 758783491);
    unsigned short newseed[3] = {1,2,3};
    unsigned short *old = seed48(newseed);
    mu_assert("seed48 old0", old[0] == 0x330e);
    mu_assert("seed48 old1", old[1] == 0x1);
    mu_assert("seed48 old2", old[2] == 0x0);
    srand48(1L);
    return 0;
}

static const char *test_arc4random_uniform_basic(void)
{
    enum { BOUND = 5, ITER = 10000 };
    unsigned counts[BOUND];
    memset(counts, 0, sizeof(counts));

    for (unsigned i = 0; i < ITER; i++) {
        unsigned v = arc4random_uniform(BOUND);
        mu_assert("in range", v < BOUND);
        counts[v]++;
    }

    unsigned expected = ITER / BOUND;
    for (unsigned i = 0; i < BOUND; i++)
        mu_assert("rough uniform",
                  counts[i] > expected - 400 && counts[i] < expected + 400);

    return 0;
}

static const char *test_forkpty_echo(void)
{
    int mfd;
    pid_t pid = forkpty(&mfd, NULL, NULL, NULL);
    mu_assert("forkpty", pid >= 0);
    if (pid == 0) {
        char buf[6] = {0};
        ssize_t n = read(0, buf, sizeof(buf)-1);
        if (n > 0)
            write(1, buf, (size_t)n);
        _exit(0);
    }
    const char *msg = "ping\n";
    mu_assert("write", write(mfd, msg, 5) == 5);
    char buf[6] = {0};
    mu_assert("read", read(mfd, buf, 5) == 5);
    mu_assert("echo", memcmp(buf, msg, 5) == 0);
    close(mfd);
    int status = 0;
    waitpid(pid, &status, 0);
    mu_assert("exit", WIFEXITED(status) && WEXITSTATUS(status) == 0);
    return 0;
}

static const char *test_tcdrain_basic(void)
{
    int m, s;
    mu_assert("openpty", openpty(&m, &s, NULL, NULL, NULL) == 0);
    const char *msg = "hi";
    mu_assert("write", write(s, msg, 2) == 2);
    mu_assert("tcdrain", tcdrain(s) == 0);
    char buf[3] = {0};
    mu_assert("read", read(m, buf, 2) == 2);
    mu_assert("match", memcmp(buf, msg, 2) == 0);
    close(m);
    close(s);
    return 0;
}

static const char *test_tcflush_basic(void)
{
    int m, s;
    mu_assert("openpty", openpty(&m, &s, NULL, NULL, NULL) == 0);
    fcntl(s, F_SETFL, O_NONBLOCK);
    const char *msg = "xx";
    mu_assert("write", write(m, msg, 2) == 2);
    mu_assert("tcflush", tcflush(s, TCIFLUSH) == 0);
    char buf[2];
    ssize_t r = read(s, buf, 2);
    mu_assert("flushed", r == -1 && errno == EAGAIN);
    close(m);
    close(s);
    return 0;
}

static const char *test_termios_speed_roundtrip(void)
{
    int m, s;
    mu_assert("openpty", openpty(&m, &s, NULL, NULL, NULL) == 0);
    struct termios t;
    mu_assert("get", tcgetattr(s, &t) == 0);
    speed_t in_orig = cfgetispeed(&t);
    speed_t out_orig = cfgetospeed(&t);
    speed_t new_in = (in_orig == B9600) ? B38400 : B9600;
    speed_t new_out = (out_orig == B9600) ? B38400 : B9600;
    mu_assert("seti", cfsetispeed(&t, new_in) == 0);
    mu_assert("seto", cfsetospeed(&t, new_out) == 0);
    mu_assert("geti", cfgetispeed(&t) == new_in);
    mu_assert("geto", cfgetospeed(&t) == new_out);
    mu_assert("apply", tcsetattr(s, TCSANOW, &t) == 0);
    struct termios verify;
    mu_assert("verify", tcgetattr(s, &verify) == 0);
    mu_assert("vi", cfgetispeed(&verify) == new_in);
    mu_assert("vo", cfgetospeed(&verify) == new_out);
    cfsetispeed(&verify, in_orig);
    cfsetospeed(&verify, out_orig);
    tcsetattr(s, TCSANOW, &verify);
    close(m);
    close(s);
    return 0;
}

static const char *test_temp_files(void)
{
    char tmpl[] = "/tmp/vlibctestXXXXXX";
    int fd = mkstemp(tmpl);
    mu_assert("mkstemp", fd >= 0);
    const char *msg = "ok";
    mu_assert("write", write(fd, msg, 2) == 2);
    close(fd);
    fd = open(tmpl, O_RDONLY);
    mu_assert("open", fd >= 0);
    close(fd);
    unlink(tmpl);

    FILE *f = tmpfile();
    mu_assert("tmpfile", f != NULL);
    mu_assert("tmpfile write", fwrite(msg, 1, 2, f) == 2);
    rewind(f);
    char buf[3] = {0};
    mu_assert("tmpfile read", fread(buf, 1, 2, f) == 2);
    mu_assert("tmpfile content", strcmp(buf, msg) == 0);
    int tmpfd = f->fd;
    fclose(f);
    mu_assert("closed", write(tmpfd, msg, 2) == -1);

    char *name = tmpnam(NULL);
    mu_assert("tmpnam", name != NULL);
    fd = open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
    mu_assert("tmpnam open", fd >= 0);
    close(fd);
    unlink(name);
    return 0;
}

static const char *test_freopen_basic(void)
{
    FILE *f = fopen("tmp_reopen", "w");
    mu_assert("fopen", f != NULL);
    mu_assert("write", fwrite("data", 1, 4, f) == 4);
    f = freopen("tmp_reopen", "r", f);
    mu_assert("freopen", f != NULL);
    char buf[5] = {0};
    mu_assert("read", fread(buf, 1, 4, f) == 4);
    fclose(f);
    unlink("tmp_reopen");
    mu_assert("content", strcmp(buf, "data") == 0);
    return 0;
}

static const char *test_fdopen_readonly(void)
{
    int fd = open("tmp_fdopen_r", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    mu_assert("open", fd >= 0);
    mu_assert("write", write(fd, "abc", 3) == 3);
    close(fd);

    fd = open("tmp_fdopen_r", O_RDONLY);
    mu_assert("open2", fd >= 0);
    FILE *f = fdopen(fd, "r");
    mu_assert("fdopen", f != NULL);
    char buf[4] = {0};
    mu_assert("read", fread(buf, 1, 3, f) == 3);
    mu_assert("content", strcmp(buf, "abc") == 0);
    fclose(f);

    fd = open("tmp_fdopen_r", O_WRONLY);
    mu_assert("open3", fd >= 0);
    errno = 0;
    f = fdopen(fd, "r");
    mu_assert("fdopen fail", f == NULL && errno == EBADF);
    close(fd);
    unlink("tmp_fdopen_r");
    return 0;
}

static const char *test_fdopen_writeonly(void)
{
    int fd = open("tmp_fdopen_w", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    mu_assert("open", fd >= 0);
    FILE *f = fdopen(fd, "w");
    mu_assert("fdopen", f != NULL);
    mu_assert("write", fwrite("hi", 1, 2, f) == 2);
    fclose(f);

    fd = open("tmp_fdopen_w", O_RDONLY);
    mu_assert("open2", fd >= 0);
    errno = 0;
    f = fdopen(fd, "w");
    mu_assert("fdopen fail", f == NULL && errno == EBADF);
    close(fd);
    unlink("tmp_fdopen_w");
    return 0;
}

static const char *test_fdopen_append(void)
{
    int fd = open("tmp_fdopen_a", O_WRONLY | O_CREAT | O_APPEND, 0644);
    mu_assert("open", fd >= 0);
    FILE *f = fdopen(fd, "a");
    mu_assert("fdopen", f != NULL);
    mu_assert("write", fwrite("x", 1, 1, f) == 1);
    fclose(f);

    fd = open("tmp_fdopen_a", O_RDONLY);
    mu_assert("open2", fd >= 0);
    errno = 0;
    f = fdopen(fd, "a");
    mu_assert("fdopen fail", f == NULL && errno == EBADF);
    close(fd);
    unlink("tmp_fdopen_a");
    return 0;
}

static const char *test_abort_fn(void)
{
    pid_t pid = fork();
    mu_assert("fork", pid >= 0);
    if (pid == 0)
        abort();
    int status = 0;
    waitpid(pid, &status, 0);
    mu_assert("abort", WIFSIGNALED(status) && WTERMSIG(status) == SIGABRT);
    return 0;
}

static volatile sig_atomic_t got_signal;

static void handle_usr1(int signo)
{
    (void)signo;
    got_signal = 1;
}

static const char *test_sigaction_install(void)
{
    got_signal = 0;
    struct sigaction sa;
    sa.sa_handler = handle_usr1;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    mu_assert("sigaction", sigaction(SIGUSR1, &sa, NULL) == 0);
    kill(getpid(), SIGUSR1);
    mu_assert("handler", got_signal == 1);
    return 0;
}

static const char *test_sigprocmask_block(void)
{
    got_signal = 0;
    struct sigaction sa;
    sa.sa_handler = handle_usr1;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    mu_assert("block", sigprocmask(SIG_BLOCK, &mask, NULL) == 0);
    kill(getpid(), SIGUSR1);
    mu_assert("blocked", got_signal == 0);
    mu_assert("unblock", sigprocmask(SIG_UNBLOCK, &mask, NULL) == 0);
    mu_assert("delivered", got_signal == 1);
    return 0;
}

static void *send_signal(void *arg)
{
    int sig = *(int *)arg;
    struct timespec ts = {0, 100000000};
    nanosleep(&ts, NULL);
    kill(getpid(), sig);
    return NULL;
}

static const char *test_sigwait_basic(void)
{
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    mu_assert("block", sigprocmask(SIG_BLOCK, &set, NULL) == 0);

    pthread_t t;
    int sig = SIGUSR1;
    pthread_create(&t, NULL, send_signal, &sig);

    int caught = 0;
    int r = sigwait(&set, &caught);
    pthread_join(t, NULL);

    sigprocmask(SIG_UNBLOCK, &set, NULL);
    mu_assert("sigwait", r == 0 && caught == SIGUSR1);
    return 0;
}

static const char *test_sigtimedwait_timeout(void)
{
    sigset_t set;
    siginfo_t info;
    struct timespec ts = {0, 100000000};
    sigemptyset(&set);
    sigaddset(&set, SIGUSR2);
int r = sigtimedwait(&set, &info, &ts);
    mu_assert("timeout", r == -1 && errno == EAGAIN);
    return 0;
}

static const char *test_sigqueue_value(void)
{
    sigset_t set;
    siginfo_t info;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    pid_t pid = fork();
    mu_assert("fork", pid >= 0);
    if (pid == 0) {
        sigprocmask(SIG_BLOCK, &set, NULL);
        sigwaitinfo(&set, &info);
        int val = ((union sigval *)&info._pad[3])->sival_int;
        _exit(val == 123 ? 0 : 1);
    }
    union sigval v; v.sival_int = 123;
    mu_assert("sigqueue", sigqueue(pid, SIGUSR1, v) == 0);
    int status = 0;
    waitpid(pid, &status, 0);
    mu_assert("sigqueue child", WIFEXITED(status) && WEXITSTATUS(status) == 0);
    return 0;
}

static const char *test_sigaltstack_basic(void)
{
    stack_t old;
    stack_t ss;
    char buf[SIGSTKSZ];
    ss.ss_sp = buf;
    ss.ss_size = sizeof(buf);
    ss.ss_flags = 0;
    mu_assert("set altstack", sigaltstack(&ss, &old) == 0);
    ss.ss_flags = SS_DISABLE;
    mu_assert("disable altstack", sigaltstack(&ss, NULL) == 0);
    (void)old;
    return 0;
}

static sigjmp_buf jbuf1;
static const char *test_sigsetjmp_restore(void)
{
    sigset_t set, cur;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    sigprocmask(SIG_BLOCK, &set, NULL);

    if (sigsetjmp(jbuf1, 1) == 0) {
        sigprocmask(SIG_UNBLOCK, &set, NULL);
        sigprocmask(SIG_BLOCK, NULL, &cur);
        mu_assert("unblock", sigismember(&cur, SIGUSR1) == 0);
        siglongjmp(jbuf1, 1);
    }

    sigprocmask(SIG_BLOCK, NULL, &cur);
    int blocked = sigismember(&cur, SIGUSR1);
    sigprocmask(SIG_UNBLOCK, &set, NULL);
    mu_assert("restored", blocked == 1);
    return 0;
}

static sigjmp_buf jbuf2;
static const char *test_sigsetjmp_nosave(void)
{
    sigset_t set, cur;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    sigprocmask(SIG_BLOCK, &set, NULL);

    if (sigsetjmp(jbuf2, 0) == 0) {
        sigprocmask(SIG_UNBLOCK, &set, NULL);
        siglongjmp(jbuf2, 1);
    }

    sigprocmask(SIG_BLOCK, NULL, &cur);
    int blocked = sigismember(&cur, SIGUSR1);
    sigprocmask(SIG_UNBLOCK, &set, NULL);
    mu_assert("not restored", blocked == 0);
    return 0;
}

static const char *test_mlock_basic(void)
{
    char buf[128];
    mu_assert("mlock", mlock(buf, sizeof(buf)) == 0);
    mu_assert("munlock", munlock(buf, sizeof(buf)) == 0);
    return 0;
}

static const char *test_mprotect_anon(void)
{
    size_t len = 4096;
    void *p = mmap(NULL, len, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANON, -1, 0);
    mu_assert("mmap", p != (void *)-1);

    ((char *)p)[0] = 'a';

    int r = mprotect(p, len, PROT_READ);
    mu_assert("mprotect read", r == 0);

    r = mprotect(p, len, PROT_READ | PROT_WRITE);
    mu_assert("mprotect rw", r == 0);

    munmap(p, len);
    return 0;
}

static const char *test_shm_basic(void)
{
    const char *name = "/vlibc_test_shm";
    int fd = shm_open(name, O_CREAT | O_RDWR, 0600);
    mu_assert("shm_open", fd >= 0);
    mu_assert("ftruncate", ftruncate(fd, 4096) == 0);
    void *p = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
                   MAP_SHARED, fd, 0);
    mu_assert("mmap", p != MAP_FAILED);
    strcpy((char *)p, "hi");
    munmap(p, 4096);
    close(fd);
    mu_assert("shm_unlink", shm_unlink(name) == 0);
    return 0;
}

static const char *test_mqueue_basic(void)
{
    const char *name = "/vlibc_test_mq";
    struct mq_attr attr = {0};
    attr.mq_maxmsg = 4;
    attr.mq_msgsize = 32;
    mqd_t mq = mq_open(name, O_CREAT | O_RDWR, 0600, &attr);
    mu_assert("mq_open", mq >= 0);

    const char *msg = "hello";
    mu_assert("mq_send", mq_send(mq, msg, strlen(msg) + 1, 1) == 0);

    char buf[32];
    unsigned prio = 0;
    ssize_t r = mq_receive(mq, buf, sizeof(buf), &prio);
    mu_assert("mq_receive", r > 0);
    mu_assert("mq_msg", strcmp(buf, msg) == 0 && prio == 1);

    mu_assert("mq_close", mq_close(mq) == 0);
    mu_assert("mq_unlink", mq_unlink(name) == 0);
    return 0;
}

static const char *test_named_semaphore_create(void)
{
    const char *name = "/vlibc_test_sem";
    sem_t *s = sem_open(name, O_CREAT | O_EXCL, 0600, 1);
    mu_assert("sem_open", s != SEM_FAILED);
    mu_assert("sem_close", sem_close(s) == 0);
    mu_assert("sem_unlink", sem_unlink(name) == 0);
    return 0;
}

static const char *test_sysv_shm_segment(void)
{
    int id = shmget(IPC_PRIVATE, 128, IPC_CREAT | 0600);
    mu_assert("shmget", id >= 0);
    void *p = shmat(id, NULL, 0);
    mu_assert("shmat", p != (void *)-1);
    strcpy((char *)p, "hi");
    mu_assert("shmdt", shmdt(p) == 0);
    mu_assert("shmctl", shmctl(id, IPC_RMID, NULL) == 0);
    return 0;
}

static const char *test_sysv_sem_basic(void)
{
    int id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);
    mu_assert("semget", id >= 0);
#ifdef _SEM_SEMUN_UNDEFINED
    union semun { int val; struct semid_ds *buf; unsigned short *array; } arg;
#else
    union semun arg;
#endif
    arg.val = 1;
    mu_assert("semctl", semctl(id, 0, SETVAL, arg) == 0);
    struct sembuf op = {0, -1, 0};
    mu_assert("semop down", semop(id, &op, 1) == 0);
    op.sem_op = 1;
    mu_assert("semop up", semop(id, &op, 1) == 0);
    mu_assert("semctl rm", semctl(id, 0, IPC_RMID) == 0);
    return 0;
}

static const char *test_ftok_unique(void)
{
    char path1[] = "/tmp/vlibc_ftok1XXXXXX";
    char path2[] = "/tmp/vlibc_ftok2XXXXXX";
    int fd1 = mkstemp(path1);
    int fd2 = mkstemp(path2);
    mu_assert("mkstemp1", fd1 >= 0);
    mu_assert("mkstemp2", fd2 >= 0);
    close(fd1);
    close(fd2);

    key_t k1 = ftok(path1, 'A');
    key_t k2 = ftok(path2, 'A');
    mu_assert("ftok1", k1 != (key_t)-1);
    mu_assert("ftok2", k2 != (key_t)-1);
    mu_assert("unique keys", k1 != k2);

    unlink(path1);
    unlink(path2);
    return 0;
}

static const char *test_atexit_handler(void)
{
    mu_assert("pipe", pipe(exit_pipe) == 0);
    pid_t pid = fork();
    mu_assert("fork", pid >= 0);
    if (pid == 0) {
        close(exit_pipe[0]);
        atexit(atexit_handler);
        exit(0);
    }
    close(exit_pipe[1]);
    char buf;
    ssize_t r = read(exit_pipe[0], &buf, 1);
    close(exit_pipe[0]);
    waitpid(pid, NULL, 0);
    mu_assert("handler ran", r == 1 && buf == 'x');

    return 0;
}

static const char *test_quick_exit_handler(void)
{
    mu_assert("pipe", pipe(exit_pipe) == 0);
    pid_t pid = fork();
    mu_assert("fork", pid >= 0);
    if (pid == 0) {
        close(exit_pipe[0]);
        at_quick_exit(atexit_handler);
        quick_exit(0);
    }
    close(exit_pipe[1]);
    char buf;
    ssize_t r = read(exit_pipe[0], &buf, 1);
    close(exit_pipe[0]);
    waitpid(pid, NULL, 0);
    mu_assert("handler ran", r == 1 && buf == 'x');

    return 0;
}

static const char *test_getcwd_chdir(void)
{
    char orig[256];
    mu_assert("getcwd orig", getcwd(orig, sizeof(orig)) != NULL);

    mu_assert("chdir root", chdir("/") == 0);
    char buf[256];
    mu_assert("getcwd root", getcwd(buf, sizeof(buf)) != NULL);
    mu_assert("root path", strcmp(buf, "/") == 0);

    mu_assert("restore", chdir(orig) == 0);
    char back[256];
    mu_assert("getcwd restore", getcwd(back, sizeof(back)) != NULL);
    mu_assert("restore path", strcmp(back, orig) == 0);

    return 0;
}

static const char *test_fchdir_basic(void)
{
    char orig[256];
    mu_assert("orig cwd", getcwd(orig, sizeof(orig)) != NULL);

    char tmpl[] = "/tmp/fcdXXXXXX";
    char *dir = mkdtemp(tmpl);
    mu_assert("mkdtemp", dir != NULL);

    int fd = open(dir, O_RDONLY);
    mu_assert("open dir", fd >= 0);

    mu_assert("fchdir", fchdir(fd) == 0);
    char buf[256];
    mu_assert("cwd dir", getcwd(buf, sizeof(buf)) != NULL);
    mu_assert("dir path", strcmp(buf, dir) == 0);

    mu_assert("restore", chdir(orig) == 0);
    close(fd);
    rmdir(dir);
    return 0;
}

static const char *test_realpath_basic(void)
{
    char cwd[256];
    mu_assert("cwd", getcwd(cwd, sizeof(cwd)) != NULL);

    char buf[256];
    mu_assert("realpath dot", realpath(".", buf) != NULL);
    mu_assert("dot eq", strcmp(buf, cwd) == 0);

    mu_assert("realpath parent", realpath("tests/..", buf) != NULL);
    mu_assert("parent eq", strcmp(buf, cwd) == 0);

    char expect[256];
    strcpy(expect, cwd);
    strcat(expect, "/tests");
    mu_assert("realpath nested", realpath("tests/../tests", buf) != NULL);
    mu_assert("nested eq", strcmp(buf, expect) == 0);

    return 0;
}

static const char *test_pathconf_basic(void)
{
    long n = pathconf("/", _PC_NAME_MAX);
    mu_assert("pathconf", n > 0);
    int fd = open("/", O_RDONLY);
    mu_assert("open root", fd >= 0);
    long nf = fpathconf(fd, _PC_NAME_MAX);
    close(fd);
    mu_assert("fpathconf", nf == n);
    return 0;
}

static const char *test_dirent(void)
{
    DIR *d = opendir("tests");
    mu_assert("opendir failed", d != NULL);
    int found = 0;
    struct dirent *e;
    while ((e = readdir(d))) {
        if (strcmp(e->d_name, "test_vlibc.c") == 0)
            found |= 1;
        if (strcmp(e->d_name, "minunit.h") == 0)
            found |= 2;
    }
    closedir(d);
    mu_assert("entries missing", found == 3);

    return 0;
}

static int walk_count;

static int nftw_counter(const char *path, const struct stat *sb, int flag,
                        struct FTW *info)
{
    (void)path; (void)sb; (void)flag; (void)info;
    walk_count++;
    return 0;
}

static int ftw_counter(const char *path, const struct stat *sb, int flag)
{
    return nftw_counter(path, sb, flag, NULL);
}

static const char *test_ftw_walk(void)
{
    char tmpl[] = "/tmp/ftwXXXXXX";
    char *dir = mkdtemp(tmpl);
    mu_assert("mkdtemp", dir != NULL);

    char buf[256];
    snprintf(buf, sizeof(buf), "%s/a", dir);
    int fd = open(buf, O_WRONLY | O_CREAT, 0600);
    mu_assert("file a", fd >= 0);
    close(fd);

    snprintf(buf, sizeof(buf), "%s/b", dir);
    mu_assert("mkdir", mkdir(buf, 0700) == 0);

    snprintf(buf, sizeof(buf), "%s/b/c", dir);
    fd = open(buf, O_WRONLY | O_CREAT, 0600);
    mu_assert("file c", fd >= 0);
    close(fd);

    walk_count = 0;
    mu_assert("nftw", nftw(dir, nftw_counter, 8, FTW_PHYS | FTW_DEPTH) == 0);
    mu_assert("count nftw", walk_count == 5);

    walk_count = 0;
    mu_assert("ftw", ftw(dir, ftw_counter, 8) == 0);
    mu_assert("count ftw", walk_count == 5);

    snprintf(buf, sizeof(buf), "%s/b/c", dir); unlink(buf);
    snprintf(buf, sizeof(buf), "%s/b", dir); rmdir(buf);
    snprintf(buf, sizeof(buf), "%s/a", dir); unlink(buf);
    rmdir(dir);
    return 0;
}

static const char *test_fts_walk(void)
{
    char tmpl[] = "/tmp/ftsXXXXXX";
    char *dir = mkdtemp(tmpl);
    mu_assert("mkdtemp", dir != NULL);

    char buf[256];
    snprintf(buf, sizeof(buf), "%s/a", dir);
    int fd = open(buf, O_WRONLY | O_CREAT, 0600);
    mu_assert("file a", fd >= 0);
    close(fd);

    snprintf(buf, sizeof(buf), "%s/b", dir);
    mu_assert("mkdir", mkdir(buf, 0700) == 0);

    snprintf(buf, sizeof(buf), "%s/b/c", dir);
    fd = open(buf, O_WRONLY | O_CREAT, 0600);
    mu_assert("file c", fd >= 0);
    close(fd);

    char *const paths[] = { dir, NULL };
    FTS *fts = fts_open(paths, FTS_PHYSICAL, NULL);
    mu_assert("fts_open", fts != NULL);

    int count = 0;
    FTSENT *ent;
    while ((ent = fts_read(fts)))
        count++;

    mu_assert("count", count == 4);
    mu_assert("fts_close", fts_close(fts) == 0);

    snprintf(buf, sizeof(buf), "%s/b/c", dir); unlink(buf);
    snprintf(buf, sizeof(buf), "%s/b", dir); rmdir(buf);
    snprintf(buf, sizeof(buf), "%s/a", dir); unlink(buf);
    rmdir(dir);
    return 0;
}

static const char *test_passwd_lookup(void)
{
    char tmpl[] = "/tmp/pwtestXXXXXX";
    int fd = mkstemp(tmpl);
    mu_assert("mkstemp", fd >= 0);
    const char data[] =
        "root:x:0:0:root:/root:/bin/sh\n"
        "alice:x:1000:1000:Alice:/home/alice:/bin/sh\n";
    mu_assert("write", write(fd, data, sizeof(data) - 1) == (ssize_t)(sizeof(data) - 1));
    close(fd);

    setenv("VLIBC_PASSWD", tmpl, 1);

    struct passwd *pw = getpwnam("alice");
    mu_assert("getpwnam", pw && pw->pw_uid == 1000 && strcmp(pw->pw_dir, "/home/alice") == 0);

    pw = getpwuid(0);
    mu_assert("getpwuid", pw && strcmp(pw->pw_name, "root") == 0);

    unsetenv("VLIBC_PASSWD");
    unlink(tmpl);
    return 0;
}

static const char *test_group_lookup(void)
{
    char tmpl[] = "/tmp/grptestXXXXXX";
    int fd = mkstemp(tmpl);
    mu_assert("mkstemp", fd >= 0);
    const char data[] =
        "root:x:0:\n"
        "staff:x:50:alice,bob\n";
    mu_assert("write", write(fd, data, sizeof(data) - 1) == (ssize_t)(sizeof(data) - 1));
    close(fd);

    setenv("VLIBC_GROUP", tmpl, 1);

    struct group *gr = getgrnam("staff");
    mu_assert("getgrnam", gr && gr->gr_gid == 50 && gr->gr_mem &&
             gr->gr_mem[0] && strcmp(gr->gr_mem[0], "alice") == 0 &&
             gr->gr_mem[1] && strcmp(gr->gr_mem[1], "bob") == 0);

    gr = getgrgid(0);
    mu_assert("getgrgid", gr && strcmp(gr->gr_name, "root") == 0);

    unsetenv("VLIBC_GROUP");
    unlink(tmpl);
    return 0;
}

static const char *test_passwd_enum(void)
{
    char tmpl[] = "/tmp/pwtenumXXXXXX";
    int fd = mkstemp(tmpl);
    mu_assert("mkstemp", fd >= 0);
    const char data[] =
        "root:x:0:0:root:/root:/bin/sh\n"
        "alice:x:1000:1000:Alice:/home/alice:/bin/sh\n";
    mu_assert("write", write(fd, data, sizeof(data) - 1) == (ssize_t)(sizeof(data) - 1));
    close(fd);

    setenv("VLIBC_PASSWD", tmpl, 1);

    setpwent();
    struct passwd *pw = getpwent();
    mu_assert("first", pw && pw->pw_uid == 0 && strcmp(pw->pw_name, "root") == 0);
    pw = getpwent();
    mu_assert("second", pw && pw->pw_uid == 1000 && strcmp(pw->pw_name, "alice") == 0);
    mu_assert("end", getpwent() == NULL);
    endpwent();

    unsetenv("VLIBC_PASSWD");
    unlink(tmpl);
    return 0;
}

static const char *test_group_enum(void)
{
    char tmpl[] = "/tmp/grpenumXXXXXX";
    int fd = mkstemp(tmpl);
    mu_assert("mkstemp", fd >= 0);
    const char data[] =
        "root:x:0:\n"
        "staff:x:50:alice,bob\n";
    mu_assert("write", write(fd, data, sizeof(data) - 1) == (ssize_t)(sizeof(data) - 1));
    close(fd);

    setenv("VLIBC_GROUP", tmpl, 1);

    setgrent();
    struct group *gr = getgrent();
    mu_assert("first", gr && gr->gr_gid == 0 && strcmp(gr->gr_name, "root") == 0);
    gr = getgrent();
    mu_assert("second", gr && gr->gr_gid == 50 && gr->gr_mem && strcmp(gr->gr_mem[1], "bob") == 0);
    mu_assert("end", getgrent() == NULL);
    endgrent();

    unsetenv("VLIBC_GROUP");
    unlink(tmpl);
    return 0;
}

static const char *test_getgrouplist_basic(void)
{
    char tmpl[] = "/tmp/glstXXXXXX";
    int fd = mkstemp(tmpl);
    mu_assert("mkstemp", fd >= 0);
    const char data[] =
        "root:x:0:\n"
        "staff:x:50:alice,bob\n"
        "extra:x:60:alice\n";
    mu_assert("write", write(fd, data, sizeof(data) - 1) ==
             (ssize_t)(sizeof(data) - 1));
    close(fd);

    setenv("VLIBC_GROUP", tmpl, 1);

    gid_t groups[4];
    int ng = 4;
    int r = getgrouplist("alice", 1000, groups, &ng);
    mu_assert("grouplist r", r >= 0 && ng == 3);
    mu_assert("g0", groups[0] == 1000);
    mu_assert("g1", groups[1] == 50);
    mu_assert("g2", groups[2] == 60);

    unsetenv("VLIBC_GROUP");
    unlink(tmpl);
    return 0;
}

static const char *test_getgrouplist_overflow(void)
{
    char tmpl[] = "/tmp/glstovXXXXXX";
    int fd = mkstemp(tmpl);
    mu_assert("mkstemp", fd >= 0);
    const char data[] =
        "root:x:0:\n"
        "staff:x:50:alice,bob\n"
        "extra:x:60:alice\n";
    mu_assert("write", write(fd, data, sizeof(data) - 1) ==
             (ssize_t)(sizeof(data) - 1));
    close(fd);

    setenv("VLIBC_GROUP", tmpl, 1);

    gid_t groups[1];
    int ng = 1;
    int r = getgrouplist("alice", 1000, groups, &ng);
    mu_assert("overflow", r == -1 && ng == 3);

    unsetenv("VLIBC_GROUP");
    unlink(tmpl);
    return 0;
}

static const char *test_getlogin_fn(void)
{
    char *name = getlogin();
    mu_assert("getlogin", name && *name != '\0');
    return 0;
}

static const char *test_crypt_des(void)
{
    const char *h = crypt("password", "ab");
    mu_assert("crypt des", strcmp(h, "abJnggxhB/yWI") == 0);
    return 0;
}

static const char *test_crypt_md5(void)
{
    const char *h = crypt("pw", "$1$aa$");
    mu_assert("crypt md5", strcmp(h, "$1$aa$2PtxCS.ei0jou2gZ339Kp0") == 0);
    return 0;
}

static const char *test_crypt_sha256(void)
{
    const char *h = crypt("pw", "$5$aa$");
    mu_assert("crypt sha256", strcmp(h, "$5$aa$mzf5CT4lKj0jBcvvaM/wyABl7jkEXQ6PNDCQjw0uBJC") == 0);
    return 0;
}

static const char *test_crypt_sha512(void)
{
    const char *h = crypt("pw", "$6$aa$");
    mu_assert("crypt sha512", strcmp(h, "$6$aa$ozCv7jillS9/rQJmK1b45G0HnIGvmtH1cIaOlMrcRZVcsh.nfXzbP1KY//LPR/ht9jXwWQtEzHAH/6vIkrhhK1") == 0);
    return 0;
}

static const char *test_wordexp_basic(void)
{
    char tmpl[] = "/tmp/wexpXXXXXX";
    char *dir = mkdtemp(tmpl);
    mu_assert("mkdtemp", dir != NULL);

    char p1[256], p2[256];
    snprintf(p1, sizeof(p1), "%s/file1.txt", dir);
    int fd = open(p1, O_WRONLY | O_CREAT, 0600);
    mu_assert("file1", fd >= 0); close(fd);

    snprintf(p2, sizeof(p2), "%s/file2.txt", dir);
    fd = open(p2, O_WRONLY | O_CREAT, 0600);
    mu_assert("file2", fd >= 0); close(fd);

    char *orig = getenv("HOME");
    orig = orig ? strdup(orig) : NULL;
    setenv("HOME", dir, 1);

    wordexp_t we;
    int r = wordexp("~/file*.txt", &we);
    mu_assert("expand", r == 0);
    mu_assert("count", we.we_wordc == 2);
    mu_assert("first", strcmp(we.we_wordv[0], p1) == 0);
    mu_assert("second", strcmp(we.we_wordv[1], p2) == 0);
    wordfree(&we);

    r = wordexp("'~/file*.txt'", &we);
    mu_assert("quote", r == 0 && we.we_wordc == 1);
    mu_assert("literal", strcmp(we.we_wordv[0], "~/file*.txt") == 0);
    wordfree(&we);

    if (orig) {
        setenv("HOME", orig, 1);
        free(orig);
    } else {
        unsetenv("HOME");
    }
    unlink(p1);
    unlink(p2);
    rmdir(dir);
    return 0;
}

static int int_cmp(const void *a, const void *b)
{
    int ia = *(const int *)a;
    int ib = *(const int *)b;
    return (ia > ib) - (ia < ib);
}

static int str_cmp(const void *a, const void *b)
{
    const char *sa = *(const char * const *)a;
    const char *sb = *(const char * const *)b;
    return strcmp(sa, sb);
}

static const char *test_qsort_int(void)
{
    int arr[] = {4, 2, 7, 1, -1};
    qsort(arr, 5, sizeof(int), int_cmp);
    int sorted[] = {-1, 1, 2, 4, 7};
    for (int i = 0; i < 5; ++i)
        mu_assert("int sort", arr[i] == sorted[i]);

    int key = 4;
    int *res = bsearch(&key, arr, 5, sizeof(int), int_cmp);
    mu_assert("bsearch int", res && *res == 4);
    return 0;
}

static const char *test_qsort_strings(void)
{
    const char *arr[] = {"pear", "apple", "orange", "banana"};
    qsort((void *)arr, 4, sizeof(char *), str_cmp);
    const char *sorted[] = {"apple", "banana", "orange", "pear"};
    for (int i = 0; i < 4; ++i)
        mu_assert("string sort", strcmp(arr[i], sorted[i]) == 0);

    const char *key = "orange";
    char **p = bsearch(&key, arr, 4, sizeof(char *), str_cmp);
    mu_assert("bsearch str", p && strcmp(*p, "orange") == 0);
    return 0;
}

static int int_cmp_dir(const void *a, const void *b, void *ctx)
{
    int dir = *(int *)ctx;
    int ia = *(const int *)a;
    int ib = *(const int *)b;
    return dir * ((ia > ib) - (ia < ib));
}

static const char *test_qsort_r_desc(void)
{
    int arr[] = {4, 2, 7, 1, -1};
    int dir = -1;
    qsort_r(arr, 5, sizeof(int), int_cmp_dir, &dir);
    int sorted[] = {7, 4, 2, 1, -1};
    for (int i = 0; i < 5; ++i)
        mu_assert("qsort_r", arr[i] == sorted[i]);
    return 0;
}

static const char *test_hsearch_basic(void)
{
    mu_assert("create", hcreate(8) == 1);
    ENTRY e = {"foo", "bar"};
    mu_assert("insert foo", hsearch(e, ENTER) != NULL);
    e.key = "baz";
    e.data = "qux";
    mu_assert("insert baz", hsearch(e, ENTER) != NULL);
    ENTRY q = {"foo", NULL};
    ENTRY *r = hsearch(q, FIND);
    mu_assert("lookup foo", r && r->data == (void *)"bar");
    hdestroy();
    return 0;
}

static int tree_sum;
static void sum_action(const void *node, VISIT v, int lvl)
{
    (void)lvl;
    if (v == postorder || v == leaf)
        tree_sum += *(const int *)node;
}

static const char *test_tsearch_basic(void)
{
    void *root = NULL;
    int vals[] = {4, 2, 7, 1, 6};
    for (int i = 0; i < 5; i++)
        mu_assert("insert", tsearch(&vals[i], &root, int_cmp) != NULL);

    int *p = tfind(&vals[2], &root, int_cmp);
    mu_assert("find 7", p && *p == 7);

    tdelete(&vals[1], &root, int_cmp);
    mu_assert("deleted", tfind(&vals[1], &root, int_cmp) == NULL);

    tree_sum = 0;
    twalk(root, sum_action);
    mu_assert("walk sum", tree_sum == 18);
    return 0;
}

static const char *test_regex_backref_basic(void)
{
    regex_t re;
    regmatch_t m[2];
    regcomp(&re, "(ab)c\\1", 0);
    int r = regexec(&re, "abcab", 2, m, 0);
    regfree(&re);
    mu_assert("regex match", r == 0);
    mu_assert("group capture", m[1].rm_so == 0 && m[1].rm_eo == 2);
    return 0;
}

static const char *test_regex_backref_fail(void)
{
    regex_t re;
    regcomp(&re, "(ab)c\\1", 0);
    int r = regexec(&re, "abcac", 0, NULL, 0);
    regfree(&re);
    mu_assert("regex nomatch", r == REG_NOMATCH);
    return 0;
}

static const char *test_regex_posix_class(void)
{
    regex_t re;
    regmatch_t m[1];
    regcomp(&re, "[[:digit:]]+", 0);
    int r = regexec(&re, "abc123def", 1, m, 0);
    regfree(&re);
    mu_assert("regex class match", r == 0);
    mu_assert("class offsets", m[0].rm_so == 3 && m[0].rm_eo == 6);
    return 0;
}

static const char *test_regex_alternation(void)
{
    regex_t re;
    regcomp(&re, "foo(bar|baz)", 0);
    int r = regexec(&re, "foobaz", 0, NULL, 0);
    regfree(&re);
    mu_assert("regex alt", r == 0);
    return 0;
}

static const char *test_regex_range(void)
{
    regex_t re;
    regcomp(&re, "a{2,3}b", 0);
    mu_assert("range1", regexec(&re, "aab", 0, NULL, 0) == 0);
    mu_assert("range2", regexec(&re, "aaab", 0, NULL, 0) == 0);
    mu_assert("range3", regexec(&re, "ab", 0, NULL, 0) == REG_NOMATCH);
    regfree(&re);
    return 0;
}

static const char *test_math_functions(void)
{
    mu_assert("fabs", fabs(-3.5) == 3.5);
    mu_assert("floor", floor(2.7) == 2.0);
    mu_assert("ceil", ceil(2.3) == 3.0);
    double l = log(5.0);
    mu_assert("exp/log", fabs(exp(l) - 5.0) < 1e-6);
    mu_assert("log/exp", fabs(log(exp(1.0)) - 1.0) < 1e-6);
    mu_assert("hypot", fabs(hypot(3.0, 4.0) - 5.0) < 1e-6);
    mu_assert("round", round(2.3) == 2.0);
    mu_assert("round half", round(2.5) == 3.0);
    mu_assert("round neg", round(-1.6) == -2.0);
    mu_assert("trunc", trunc(2.9) == 2.0);
    mu_assert("trunc neg", trunc(-2.9) == -2.0);
    return 0;
}

static const char *test_complex_cabs_cexp(void)
{
    double_complex z = {3.0, 4.0};
    mu_assert("cabs", fabs(cabs(z) - 5.0) < 1e-6);

    double_complex i_pi = {0.0, 3.14159265358979323846};
    double_complex r = cexp(i_pi);
    mu_assert("cexp real", fabs(r.real + 1.0) < 1e-6);
    mu_assert("cexp imag", fabs(r.imag) < 1e-6);
    return 0;
}

static const char *test_abs_div_functions(void)
{
    mu_assert("abs pos", abs(5) == 5);
    mu_assert("abs neg", abs(-5) == 5);
    mu_assert("labs pos", labs(7L) == 7L);
    mu_assert("labs neg", labs(-7L) == 7L);
    mu_assert("llabs pos", llabs(9LL) == 9LL);
    mu_assert("llabs neg", llabs(-9LL) == 9LL);

    div_t di = div(7, 3);
    mu_assert("div quot", di.quot == 2);
    mu_assert("div rem", di.rem == 1);

    ldiv_t ld = ldiv(8L, 3L);
    mu_assert("ldiv quot", ld.quot == 2L);
    mu_assert("ldiv rem", ld.rem == 2L);

    lldiv_t lld = lldiv(-10LL, 3LL);
    mu_assert("lldiv quot", lld.quot == -3LL);
    mu_assert("lldiv rem", lld.rem == -1LL);

    return 0;
}

static const char *test_fp_checks(void)
{
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double ninf = -1.0 / zero;
    double nanv = zero / zero;

    mu_assert("isinf pos", isinf(inf));
    mu_assert("isinf neg", isinf(ninf));
    mu_assert("!isinf", !isinf(1.5));
    mu_assert("isnan", isnan(nanv));
    mu_assert("!isnan", !isnan(inf));
    mu_assert("isfinite", isfinite(2.0));
    mu_assert("isfinite zero", isfinite(0.0));
    mu_assert("!isfinite nan", !isfinite(nanv));
    mu_assert("!isfinite inf", !isfinite(inf));
    return 0;
}

static const char *test_fenv_rounding(void)
{
    int orig = fegetround();
    mu_assert("set down", fesetround(FE_DOWNWARD) == 0);
    mu_assert("down mode", fegetround() == FE_DOWNWARD);
    double d = __builtin_nearbyint(1.3);
    mu_assert("round down", d == 1.0);
    mu_assert("set up", fesetround(FE_UPWARD) == 0);
    mu_assert("up mode", fegetround() == FE_UPWARD);
    d = __builtin_nearbyint(1.3);
    mu_assert("round up", d == 2.0);
    mu_assert("restore", fesetround(orig) == 0);
    return 0;
}

static void encode_vis(const char *src, char *dst, int flags)
{
    while (*src) {
        dst += vis(dst, (unsigned char)*src, flags, src[1]);
        src++;
    }
    *dst = '\0';
}

static int decode_vis(const char *src, char *dst)
{
    int st = 0;
    while (*src) {
        int r = unvis(dst, (unsigned char)*src++, &st, 0);
        if (r == UNVIS_VALID)
            dst++;
        else if (r == UNVIS_SYNBAD)
            return -1;
    }
    if (unvis(dst, '\0', &st, UNVIS_END) == UNVIS_VALID)
        dst++;
    *dst = '\0';
    return 0;
}

static const char *test_vis_roundtrip(void)
{
    const char *src = "hello\n\tworld";
    char enc[64];
    encode_vis(src, enc, VIS_CSTYLE);
    mu_assert("encode", strcmp(enc, "hello\\n\\tworld") == 0);
    char dec[64];
    mu_assert("decode", decode_vis(enc, dec) == 0);
    mu_assert("roundtrip", strcmp(dec, src) == 0);
    return 0;
}

static const char *test_nvis_basic(void)
{
    char buf[8];
    int n = nvis(buf, sizeof(buf), '\n', VIS_CSTYLE, 0);
    mu_assert("nvis len", n == 2);
    mu_assert("nvis text", strcmp(buf, "\\n") == 0);
    return 0;
}

static const char *test_getopt_basic(void)
{
    char *argv[] = {"prog", "-f", "-a", "val", "rest", NULL};
    int argc = 5;
    int flag = 0;
    char *arg = NULL;
    optind = 1;
    opterr = 0;
    int c;
    while ((c = getopt(argc, argv, "fa:")) != -1) {
        switch (c) {
        case 'f':
            flag = 1;
            break;
        case 'a':
            arg = optarg;
            break;
        default:
            return "unexpected opt";
        }
    }
    mu_assert("flag", flag == 1);
    mu_assert("arg", arg && strcmp(arg, "val") == 0);
    mu_assert("optind", optind == 4);
    mu_assert("rest", strcmp(argv[optind], "rest") == 0);
    return 0;
}

static const char *test_getopt_missing(void)
{
    char *argv[] = {"prog", "-a", NULL};
    int argc = 2;
    optind = 1;
    opterr = 0;
    int r = getopt(argc, argv, "a:");
    mu_assert("missing ret", r == '?');
    mu_assert("optopt", optopt == 'a');
    mu_assert("index", optind == 2);
    return 0;
}

static const char *test_dlopen_basic(void)
{
    void *h = dlopen("tests/plugin.so", RTLD_NOW);
    mu_assert("dlopen", h != NULL);
    int (*val)(void) = dlsym(h, "plugin_value");
    mu_assert("dlsym", val != NULL);
    mu_assert("call", val() == 123);
    mu_assert("dlclose", dlclose(h) == 0);
    return 0;
}

static const char *test_dladdr_basic(void)
{
    void *h = dlopen("tests/plugin.so", RTLD_NOW);
    mu_assert("dlopen", h != NULL);
    int (*val)(void) = dlsym(h, "plugin_value");
    mu_assert("dlsym", val != NULL);
    Dl_info info;
    memset(&info, 0, sizeof(info));
    mu_assert("dladdr", dladdr((void *)val, &info) == 1);
    mu_assert("symbol", info.dli_sname && strcmp(info.dli_sname, "plugin_value") == 0);
    mu_assert("addr", info.dli_saddr == (void *)val);
    mu_assert("file", info.dli_fname && strstr(info.dli_fname, "plugin.so"));
    dlclose(h);
    return 0;
}

static const char *test_getopt_long_missing(void)
{
    char *argv[] = {"prog", "--bar", NULL};
    int argc = 2;
    struct option longopts[] = {
        {"bar", required_argument, NULL, 'b'},
        {0, 0, 0, 0}
    };
    optind = 1;
    opterr = 0;
    int r = getopt_long(argc, argv, "b:", longopts, NULL);
    mu_assert("missing ret", r == '?');
    mu_assert("optopt", optopt == 'b');
    mu_assert("index", optind == 2);
    return 0;
}

static const char *test_getopt_long_basic(void)
{
    char *argv[] = {"prog", "--foo", "--bar=val", "rest", NULL};
    int argc = 4;
    int foo = 0;
    char *bar = NULL;
    struct option longopts[] = {
        {"foo", no_argument, &foo, 1},
        {"bar", required_argument, NULL, 'b'},
        {0, 0, 0, 0}
    };
    optind = 1;
    opterr = 0;
    int c;
    while ((c = getopt_long(argc, argv, "b:", longopts, NULL)) != -1) {
        switch (c) {
        case 0:
            break;
        case 'b':
            bar = optarg;
            break;
        default:
            return "unexpected long opt";
        }
    }
    mu_assert("foo", foo == 1);
    mu_assert("bar", bar && strcmp(bar, "val") == 0);
    mu_assert("optind", optind == 3);
    mu_assert("rest", strcmp(argv[optind], "rest") == 0);

    return 0;
}

static const char *test_getopt_long_only_missing(void)
{
    char *argv[] = {"prog", "-bar", NULL};
    int argc = 2;
    struct option longopts[] = {
        {"bar", required_argument, NULL, 'b'},
        {0, 0, 0, 0}
    };
    optind = 1;
    opterr = 0;
    int r = getopt_long_only(argc, argv, "b:", longopts, NULL);
    mu_assert("missing ret", r == '?');
    mu_assert("optopt", optopt == 'b');
    mu_assert("index", optind == 2);
    return 0;
}

static const char *test_getopt_long_only_basic(void)
{
    char *argv[] = {"prog", "-foo", "-bar=val", "rest", NULL};
    int argc = 4;
    int foo = 0;
    char *bar = NULL;
    struct option longopts[] = {
        {"foo", no_argument, &foo, 1},
        {"bar", required_argument, NULL, 'b'},
        {0, 0, 0, 0}
    };
    optind = 1;
    opterr = 0;
    int c;
    while ((c = getopt_long_only(argc, argv, "b:", longopts, NULL)) != -1) {
        switch (c) {
        case 0:
            break;
        case 'b':
            bar = optarg;
            break;
        default:
            return "unexpected long opt";
        }
    }
    mu_assert("foo", foo == 1);
    mu_assert("bar", bar && strcmp(bar, "val") == 0);
    mu_assert("optind", optind == 3);
    mu_assert("rest", strcmp(argv[optind], "rest") == 0);

    return 0;
}

static const char *test_getsubopt_basic(void)
{
    char opts[] = "foo=1,bar,baz=2";
    char *p = opts;
    char *val = NULL;
    char *tokens[] = {"foo", "bar", "baz", NULL};

    int r = getsubopt(&p, tokens, &val);
    mu_assert("foo index", r == 0);
    mu_assert("foo val", val && strcmp(val, "1") == 0);

    r = getsubopt(&p, tokens, &val);
    mu_assert("bar index", r == 1);
    mu_assert("bar val", val == NULL);

    r = getsubopt(&p, tokens, &val);
    mu_assert("baz index", r == 2);
    mu_assert("baz val", val && strcmp(val, "2") == 0);

    return 0;
}

static const char *test_getsubopt_unknown(void)
{
    char opts[] = "foo=1,unknown";
    char *p = opts;
    char *val = NULL;
    char *tokens[] = {"foo", NULL};

    int r = getsubopt(&p, tokens, &val);
    mu_assert("known index", r == 0);
    mu_assert("known val", val && strcmp(val, "1") == 0);

    r = getsubopt(&p, tokens, &val);
    mu_assert("unknown ret", r == -1);
    mu_assert("unknown val", val == NULL);

    return 0;
}

static const char *run_tests(const char *category)
{
    static const struct test_case tests[] = {
        MEM_TEST(test_malloc),
        MEM_TEST(test_malloc_reuse),
        MEM_TEST(test_reallocf_fail),
        MEM_TEST(test_posix_memalign_basic),
        MEM_TEST(test_posix_memalign),
        MEM_TEST(test_aligned_alloc),
        MEM_TEST(test_posix_memalign_overflow),
        MEM_TEST(test_reallocarray_overflow),
        MEM_TEST(test_reallocarray_basic),
        MEM_TEST(test_recallocarray_grow),
        MEM_TEST(test_memory_ops),
        IO_TEST(test_io),
        IO_TEST(test_lseek_dup),
        IO_TEST(test_lseek_negative_offset),
        IO_TEST(test_pread_pwrite),
        IO_TEST(test_preadv_pwritev),
        IO_TEST(test_readv_writev),
        IO_TEST(test_sendfile_copy),
#ifdef __NetBSD__
        NET_TEST(test_sendfile_socket),
#endif
        IO_TEST(test_dup3_cloexec),
        IO_TEST(test_pipe2_cloexec),
        NET_TEST(test_byte_order),
        IO_TEST(test_isatty_stdin),
        IO_TEST(test_ttyname_dev_tty),
        IO_TEST(test_ttyname_openpty),
        NET_TEST(test_socket),
        NET_TEST(test_socketpair_basic),
        NET_TEST(test_socket_addresses),
        NET_TEST(test_sendmsg_recvmsg),
        NET_TEST(test_udp_send_recv),
        NET_TEST(test_inet_pton_ntop),
        NET_TEST(test_inet_aton_ntoa),
        IO_TEST(test_errno_open),
        IO_TEST(test_errno_stat),
        IO_TEST(test_stat_wrappers),
        IO_TEST(test_truncate_resize),
        IO_TEST(test_posix_fallocate_basic),
        IO_TEST(test_posix_fadvise_basic),
        IO_TEST(test_posix_fadvise_invalid),
        MEM_TEST(test_posix_madvise_basic),
        IO_TEST(test_link_readlink),
        IO_TEST(test_at_wrappers_basic),
        IO_TEST(test_fsync_basic),
        IO_TEST(test_fdatasync_basic),
        IO_TEST(test_sync_basic),
        REGISTER_TEST("string", test_string_helpers),
        REGISTER_TEST("string", test_string_casecmp),
        REGISTER_TEST("string", test_strlcpy_cat),
        REGISTER_TEST("string", test_stpcpy_functions),
        REGISTER_TEST("string", test_memccpy_mempcpy),
        REGISTER_TEST("string", test_memccpy_zero),
        REGISTER_TEST("string", test_strndup_basic),
        REGISTER_TEST("string", test_strcoll_xfrm),
        REGISTER_TEST("string", test_wcscoll_xfrm),
        REGISTER_TEST("string", test_ctype_extra),
        REGISTER_TEST("string", test_widechar_basic),
        REGISTER_TEST("string", test_widechar_conv),
        REGISTER_TEST("string", test_widechar_width),
        REGISTER_TEST("string", test_wctype_checks),
        REGISTER_TEST("string", test_wmem_ops),
        REGISTER_TEST("string", test_wchar_search),
        REGISTER_TEST("string", test_wmemstream_basic),
        REGISTER_TEST("string", test_fopencookie_basic),
        REGISTER_TEST("string", test_iconv_ascii_roundtrip),
        REGISTER_TEST("string", test_iconv_invalid_byte),
        REGISTER_TEST("string", test_strtok_basic),
        REGISTER_TEST("string", test_strtok_r_basic),
        REGISTER_TEST("string", test_strsep_basic),
        REGISTER_TEST("string", test_wcstok_basic),
        REGISTER_TEST("string", test_printf_functions),
        REGISTER_TEST("string", test_dprintf_functions),
        REGISTER_TEST("string", test_scanf_functions),
        REGISTER_TEST("string", test_vscanf_variants),
        IO_TEST(test_fseek_rewind),
        IO_TEST(test_fgetpos_fsetpos),
        IO_TEST(test_fgetc_fputc),
        IO_TEST(test_fgets_fputs),
        IO_TEST(test_fgetwc_fputwc),
        IO_TEST(test_getwc_putwc),
        IO_TEST(test_getline_various),
        IO_TEST(test_getdelim_various),
        IO_TEST(test_fflush),
        IO_TEST(test_feof_flag),
        IO_TEST(test_ferror_flag),
        THREAD_TEST(test_pthread_create_join),
        THREAD_TEST(test_pthread),
        THREAD_TEST(test_pthread_detach),
        THREAD_TEST(test_pthread_exit),
        THREAD_TEST(test_pthread_cancel),
        THREAD_TEST(test_pthread_tls),
        THREAD_TEST(test_pthread_mutexattr),
        THREAD_TEST(test_pthread_mutex_recursive),
        THREAD_TEST(test_pthread_attr_basic),
        THREAD_TEST(test_pthread_rwlock),
        THREAD_TEST(test_pthread_barrier),
        THREAD_TEST(test_pthread_spinlock),
        THREAD_TEST(test_pthread_cond_signal),
        THREAD_TEST(test_pthread_cond_broadcast),
        THREAD_TEST(test_semaphore_basic),
        THREAD_TEST(test_semaphore_trywait),
        THREAD_TEST(test_select_pipe),
        THREAD_TEST(test_poll_pipe),
        TIME_TEST(test_sleep_functions),
        TIME_TEST(test_clock_nanosleep_basic),
        THREAD_TEST(test_sched_yield_basic),
        THREAD_TEST(test_sched_yield_loop),
        THREAD_TEST(test_priority_wrappers),
        THREAD_TEST(test_sched_get_set_scheduler),
        TIME_TEST(test_timer_basic),
        TIME_TEST(test_clock_settime_priv),
        TIME_TEST(test_getrusage_self),
        TIME_TEST(test_times_self),
        TIME_TEST(test_getloadavg_basic),
        TIME_TEST(test_timespec_get_basic),
        TIME_TEST(test_strftime_basic),
        TIME_TEST(test_strftime_extended),
        TIME_TEST(test_wcsftime_basic),
        TIME_TEST(test_wcsftime_extended),
        TIME_TEST(test_strptime_basic),
        TIME_TEST(test_time_conversions),
        TIME_TEST(test_time_r_conversions),
        TIME_TEST(test_timegm_known_values),
        TIME_TEST(test_asctime_r_threadsafe),
        TIME_TEST(test_difftime_basic),
        TIME_TEST(test_tz_positive),
        TIME_TEST(test_tz_negative),
        TIME_TEST(test_tz_mktime_roundtrip),
        TIME_TEST(test_tz_ctime),
        REGISTER_TEST("env", test_environment),
        REGISTER_TEST("env", test_clearenv_fn),
        REGISTER_TEST("env", test_env_init_clearenv),
        REGISTER_TEST("env", test_locale_from_env),
        REGISTER_TEST("env", test_locale_objects),
        REGISTER_TEST("env", test_gethostname_fn),
        REGISTER_TEST("env", test_uname_fn),
        REGISTER_TEST("env", test_confstr_path),
        REGISTER_TEST("env", test_progname_setget),
        REGISTER_TEST("env", test_error_reporting),
        REGISTER_TEST("env", test_warn_functions),
        REGISTER_TEST("env", test_err_functions),
        REGISTER_TEST("env", test_strsignal_names),
        PROC_TEST(test_process_group_wrappers),
        PROC_TEST(test_system_fn),
        PROC_TEST(test_system_interrupted),
        PROC_TEST(test_execv_fn),
        PROC_TEST(test_execl_fn),
        PROC_TEST(test_execlp_fn),
        PROC_TEST(test_execle_fn),
        PROC_TEST(test_execvp_fn),
        PROC_TEST(test_fexecve_fn),
        PROC_TEST(test_posix_spawn_fn),
        REGISTER_TEST("default", test_posix_spawn_actions),
        REGISTER_TEST("default", test_posix_spawn_sigmask),
        REGISTER_TEST("default", test_posix_spawn_pgroup),
        PROC_TEST(test_popen_fn),
        UTIL_TEST(test_rand_fn),
        UTIL_TEST(test_rand48_fn),
        UTIL_TEST(test_arc4random_uniform_basic),
        PROC_TEST(test_forkpty_echo),
        PROC_TEST(test_tcdrain_basic),
        PROC_TEST(test_tcflush_basic),
        PROC_TEST(test_termios_speed_roundtrip),
        FS_TEST(test_temp_files),
        FS_TEST(test_freopen_basic),
        REGISTER_TEST("fdopen", test_fdopen_readonly),
        REGISTER_TEST("fdopen", test_fdopen_writeonly),
        REGISTER_TEST("fdopen", test_fdopen_append),
        PROC_TEST(test_abort_fn),
        PROC_TEST(test_sigaction_install),
        PROC_TEST(test_sigprocmask_block),
        PROC_TEST(test_sigwait_basic),
        PROC_TEST(test_sigtimedwait_timeout),
        PROC_TEST(test_sigqueue_value),
        PROC_TEST(test_sigaltstack_basic),
        PROC_TEST(test_sigsetjmp_restore),
        PROC_TEST(test_sigsetjmp_nosave),
        MEM_TEST(test_mlock_basic),
        MEM_TEST(test_mprotect_anon),
        MEM_TEST(test_shm_basic),
        MEM_TEST(test_sysv_shm_segment),
        MEM_TEST(test_mqueue_basic),
        MEM_TEST(test_named_semaphore_create),
        MEM_TEST(test_sysv_sem_basic),
        MEM_TEST(test_ftok_unique),
        PROC_TEST(test_atexit_handler),
        PROC_TEST(test_quick_exit_handler),
        FS_TEST(test_getcwd_chdir),
        FS_TEST(test_fchdir_basic),
        FS_TEST(test_realpath_basic),
        FS_TEST(test_pathconf_basic),
        ENV_TEST(test_passwd_lookup),
        ENV_TEST(test_group_lookup),
        ENV_TEST(test_passwd_enum),
        ENV_TEST(test_group_enum),
        ENV_TEST(test_getgrouplist_basic),
        ENV_TEST(test_getgrouplist_overflow),
        ENV_TEST(test_getlogin_fn),
        CRYPTO_TEST(test_crypt_des),
        CRYPTO_TEST(test_crypt_md5),
        CRYPTO_TEST(test_crypt_sha256),
        CRYPTO_TEST(test_crypt_sha512),
        FS_TEST(test_wordexp_basic),
        FS_TEST(test_dirent),
        FS_TEST(test_ftw_walk),
        FS_TEST(test_fts_walk),
        UTIL_TEST(test_qsort_int),
        UTIL_TEST(test_qsort_strings),
        UTIL_TEST(test_qsort_r_desc),
        UTIL_TEST(test_hsearch_basic),
        UTIL_TEST(test_tsearch_basic),
        REGEX_TEST(test_regex_backref_basic),
        REGEX_TEST(test_regex_backref_fail),
        REGEX_TEST(test_regex_posix_class),
        REGEX_TEST(test_regex_alternation),
        REGEX_TEST(test_regex_range),
        MATH_TEST(test_math_functions),
        MATH_TEST(test_complex_cabs_cexp),
        MATH_TEST(test_abs_div_functions),
        UTIL_TEST(test_vis_roundtrip),
        UTIL_TEST(test_nvis_basic),
        MATH_TEST(test_fp_checks),
        MATH_TEST(test_fenv_rounding),
        UTIL_TEST(test_getopt_basic),
        UTIL_TEST(test_getopt_missing),
        UTIL_TEST(test_dlopen_basic),
        UTIL_TEST(test_dladdr_basic),
        UTIL_TEST(test_getopt_long_missing),
        UTIL_TEST(test_getopt_long_basic),
        UTIL_TEST(test_getopt_long_only_missing),
        UTIL_TEST(test_getopt_long_only_basic),
        UTIL_TEST(test_getsubopt_basic),
        UTIL_TEST(test_getsubopt_unknown),
    };
    for (size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); i++) {
        if (category == NULL || strcmp(tests[i].category, category) == 0) {
            const char *msg = tests[i].func();
            if (msg)
                return msg;
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    const char *category = NULL;
    if (argc > 1)
        category = argv[1];
    else
        category = getenv("TEST_GROUP");

    const char *result = run_tests(category);
    if (result)
        printf("%s\n", result);
    else
        printf("ALL TESTS PASSED\n");
    printf("Tests run: %d\n", tests_run);
    return result != 0;
}
