#include "minunit.h"
#include "../include/memory.h"
#include "../include/io.h"
#include "../include/sys/socket.h"
#include "../include/sys/stat.h"

#include <fcntl.h>
#include <string.h>
#include <unistd.h>

int tests_run = 0;

static const char *test_malloc(void)
{
    void *p = malloc(16);
    mu_assert("malloc returned NULL", p != NULL);
    memset(p, 0xAA, 16);
    free(p);
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

static const char *all_tests(void)
{
    mu_run_test(test_malloc);
    mu_run_test(test_io);
    mu_run_test(test_socket);
    mu_run_test(test_stat_wrappers);
    mu_run_test(test_string_helpers);
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
