#include "minunit.h"
#include "../include/memory.h"
#include "../include/io.h"
#include "../include/sys/socket.h"
#include "../include/sys/stat.h"
#include "../include/stdio.h"
#include "../include/pthread.h"
#include "../include/dirent.h"

#include <fcntl.h>
#include "../include/string.h"
#include "../include/stdlib.h"
#include "../include/env.h"
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include "../include/time.h"

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

static const char *test_system_fn(void)
{
    int r = system("true");
    mu_assert("system true", r == 0);
    r = system("exit 7");
    mu_assert("system exit code", (r >> 8) == 7);
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

static const char *all_tests(void)
{
    mu_run_test(test_malloc);
    mu_run_test(test_malloc_reuse);
    mu_run_test(test_memory_ops);
    mu_run_test(test_io);
    mu_run_test(test_lseek_dup);
    mu_run_test(test_socket);
    mu_run_test(test_errno_open);
    mu_run_test(test_errno_stat);
    mu_run_test(test_stat_wrappers);
    mu_run_test(test_string_helpers);
    mu_run_test(test_printf_functions);
    mu_run_test(test_pthread);
    mu_run_test(test_sleep_functions);
    mu_run_test(test_environment);
    mu_run_test(test_system_fn);
    mu_run_test(test_rand_fn);
    mu_run_test(test_dirent);
    mu_run_test(test_qsort_int);
    mu_run_test(test_qsort_strings);

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
