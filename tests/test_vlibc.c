#include "minunit.h"
#include "../include/memory.h"
#include "../include/io.h"
#include "../include/sys/socket.h"
#include "../include/sys/stat.h"
#include "../include/stdio.h"
#include "../include/pthread.h"

#include <fcntl.h>
#include "../include/string.h"
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

/* use host printf for test output */
int printf(const char *fmt, ...);

int tests_run = 0;

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

static const char *test_socket(void)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    mu_assert("socket creation failed", fd >= 0);
    if (fd >= 0)
        close(fd);
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

static const char *test_pthread(void)
{
    pthread_t t;
    int val = 0;
    int r = pthread_create(&t, NULL, thread_fn, &val);
    mu_assert("pthread_create", r == 0);
    void *ret = NULL;
    pthread_join(&t, &ret);
    mu_assert("thread retval", ret == (void *)123);
    mu_assert("shared value", val == 42);
    return 0;
}

static const char *all_tests(void)
{
    mu_run_test(test_malloc);
    mu_run_test(test_memory_ops);
    mu_run_test(test_io);
    mu_run_test(test_socket);
    mu_run_test(test_errno_open);
    mu_run_test(test_errno_stat);
    mu_run_test(test_stat_wrappers);
    mu_run_test(test_string_helpers);
    mu_run_test(test_printf_functions);
    mu_run_test(test_pthread);

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
