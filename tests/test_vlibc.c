#include "minunit.h"
#include "../include/memory.h"
#include "../include/io.h"
#include "../include/sys/socket.h"
#include "../include/sys/stat.h"
#include "../include/stdio.h"
#include "../include/pthread.h"
#include "../include/sys/select.h"
#include "../include/poll.h"
#include "../include/dirent.h"
#include "../include/vlibc.h"
#include "../include/dlfcn.h"

#include <fcntl.h>
#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif
#include "../include/string.h"
#include "../include/stdlib.h"
#include "../include/wchar.h"
#include "../include/env.h"
#include "../include/process.h"
#include "../include/getopt.h"
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include "../include/time.h"
#include "../include/sys/mman.h"

/* use host printf for test output */
int printf(const char *fmt, ...);

int tests_run = 0;

static int exit_pipe[2];

static void atexit_handler(void)
{
    write(exit_pipe[1], "x", 1);
}

static uint16_t bswap16(uint16_t v)
{
    return (uint16_t)((v << 8) | (v >> 8));
}

static uint32_t bswap32(uint32_t v)
{
    return __builtin_bswap32(v);
}

#define htons(x) bswap16(x)
#define ntohs(x) bswap16(x)
#define htonl(x) bswap32(x)

static void *thread_fn(void *arg)
{
    int *p = arg;
    *p = 42;
    return (void *)123;
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

static const char *test_socket(void)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    mu_assert("socket creation failed", fd >= 0);
    if (fd >= 0)
        close(fd);
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
    mu_assert("strtod basic", strtod("2.5", &end) == 2.5 && *end == '\0');
    mu_assert("strtod exp", strtod("1e2", &end) == 100.0 && *end == '\0');
    mu_assert("atof", atof("-3.0") == -3.0);

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

static const char *test_printf_functions(void)
{
    char buf[32];
    int n = snprintf(buf, sizeof(buf), "v=%d %s", 42, "ok");
    mu_assert("snprintf len", n == (int)strlen("v=42 ok"));
    mu_assert("snprintf buf", strcmp(buf, "v=42 ok") == 0);

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
    mu_assert("join fails", pthread_join(t, NULL) == -1);

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

    char *s = ctime(&t);
    mu_assert("ctime", strcmp(s, "Tue Nov 14 22:13:20 2023\n") == 0);
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

static const char *test_system_fn(void)
{
    int r = system("true");
    mu_assert("system true", r == 0);
    r = system("exit 7");
    mu_assert("system exit code", (r >> 8) == 7);
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

static const char *all_tests(void)
{
    mu_run_test(test_malloc);
    mu_run_test(test_malloc_reuse);
    mu_run_test(test_memory_ops);
    mu_run_test(test_io);
    mu_run_test(test_lseek_dup);
    mu_run_test(test_dup3_cloexec);
    mu_run_test(test_pipe2_cloexec);
    mu_run_test(test_socket);
    mu_run_test(test_udp_send_recv);
    mu_run_test(test_errno_open);
    mu_run_test(test_errno_stat);
    mu_run_test(test_stat_wrappers);
    mu_run_test(test_string_helpers);
    mu_run_test(test_widechar_basic);
    mu_run_test(test_strtok_basic);
    mu_run_test(test_strtok_r_basic);
    mu_run_test(test_printf_functions);
    mu_run_test(test_fseek_rewind);
    mu_run_test(test_fgetc_fputc);
    mu_run_test(test_fgets_fputs);
    mu_run_test(test_fflush);
    mu_run_test(test_pthread);
    mu_run_test(test_pthread_detach);
    mu_run_test(test_select_pipe);
    mu_run_test(test_poll_pipe);
    mu_run_test(test_sleep_functions);
    mu_run_test(test_strftime_basic);
    mu_run_test(test_time_conversions);
    mu_run_test(test_environment);
    mu_run_test(test_system_fn);
    mu_run_test(test_execvp_fn);
    mu_run_test(test_popen_fn);
    mu_run_test(test_rand_fn);
    mu_run_test(test_abort_fn);
    mu_run_test(test_mprotect_anon);
    mu_run_test(test_atexit_handler);
    mu_run_test(test_dirent);
    mu_run_test(test_qsort_int);
    mu_run_test(test_qsort_strings);
    mu_run_test(test_getopt_basic);
    mu_run_test(test_getopt_missing);
    mu_run_test(test_dlopen_basic);
    mu_run_test(test_getopt_long_missing);
    mu_run_test(test_getopt_long_basic);

    return 0;
}

int main(void)
{
    const char *result = all_tests();
    if (result)
        printf("%s\n", result);
    else
        printf("ALL TESTS PASSED\n");
    printf("Tests run: %d\n", tests_run);
    return result != 0;
}
