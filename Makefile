# Simple build rules for vlibc

PREFIX ?= /usr/local
CC ?= cc
CFLAGS ?= -O2 -Wall -Wextra -fno-stack-protector -fno-builtin -Iinclude
CFLAGS += -std=c11
AR ?= ar
ARCH ?= $(shell uname -m)

# Detect availability of sbrk
HAVE_SBRK := $(shell printf '#include <unistd.h>\nint main(){void *p=sbrk(0);return 0;}' | $(CC) -x c - -Werror -c -o /dev/null 2>/dev/null && echo 1 || echo 0)
ifeq ($(HAVE_SBRK),1)
CFLAGS += -DHAVE_SBRK
endif

# Detect optional syscalls used by vlibc
HAVE_ACCEPT4 := $(shell printf '#include <sys/syscall.h>\nint main(){return SYS_accept4;}' | $(CC) -x c - -Werror -c -o /dev/null 2>/dev/null && echo 1 || echo 0)
ifeq ($(HAVE_ACCEPT4),1)
CFLAGS += -DVLIBC_HAVE_ACCEPT4=1
endif
HAVE_PIPE2 := $(shell printf '#include <sys/syscall.h>\nint main(){return SYS_pipe2;}' | $(CC) -x c - -Werror -c -o /dev/null 2>/dev/null && echo 1 || echo 0)
ifeq ($(HAVE_PIPE2),1)
CFLAGS += -DVLIBC_HAVE_PIPE2=1
endif
HAVE_DUP3 := $(shell printf '#include <sys/syscall.h>\nint main(){return SYS_dup3;}' | $(CC) -x c - -Werror -c -o /dev/null 2>/dev/null && echo 1 || echo 0)
ifeq ($(HAVE_DUP3),1)
CFLAGS += -DVLIBC_HAVE_DUP3=1
endif

TARGET_OS ?= $(OS)
ifeq ($(TARGET_OS),)
TARGET_OS := $(shell uname -s)
endif

ifneq (,$(filter $(TARGET_OS),FreeBSD NetBSD OpenBSD DragonFly))
CFLAGS += -D__BSD_VISIBLE
endif

SYS_SRC := src/syscall.c
ifneq (,$(filter $(TARGET_OS),FreeBSD NetBSD OpenBSD DragonFly))
SYS_SRC := src/arch/bsd/syscall.c
else ifeq ($(TARGET_OS),Windows_NT)
SYS_SRC := src/arch/win32/syscall.c
endif

# Use the system implementations of select(2) and poll(2) on the BSD
# family. These wrappers are only needed on Linux where direct
# syscalls are issued.
SELECT_SRC := src/select.c
ifneq (,$(filter $(TARGET_OS),FreeBSD NetBSD OpenBSD DragonFly))
SELECT_SRC :=
endif

SRC := \
    src/errno.c \
    src/error.c \
    src/init.c \
    src/io.c \
    src/stdio.c \
    src/printf.c \
    src/scanf.c \
    src/memory.c \
    src/memory_ops.c \
    src/atexit.c \
    src/process.c \
    src/signal.c \
    src/abort.c \
    src/system.c \
    src/daemon.c \
    src/string.c \
    src/string_extra.c \
    src/strndup.c \
    src/strerror_r.c \
    src/strto.c \
    src/rand.c \
    src/arc4random.c \
    src/socket.c \
    src/socketpair.c \
    src/setsockopt.c \
    src/socket_msg.c \
    src/getsockopt.c \
    src/netdb.c \
    src/inet_pton.c \
    src/inet_ntop.c \
    src/fd.c \
    src/fcntl.c \
    src/isatty.c \
    src/termios.c \
    src/file.c \
    src/file_perm.c \
    src/truncate.c \
    src/dir.c \
    src/access.c \
    src/getcwd.c \
    src/realpath.c \
    src/path.c \
    $(SYS_SRC) \
    src/mmap.c \
    src/env.c \
    src/hostname.c \
    src/uname.c \
    src/sleep.c \
    src/clock_gettime.c \
    src/time.c \
    src/time_conv.c \
    src/time_r.c \
    src/itimer.c \
    src/strftime.c \
    src/strptime.c \
    src/stat.c \
    src/statvfs.c \
    src/utime.c \
    src/pthread.c \
    src/semaphore.c \
    src/dirent.c \
    src/default_shell.c \
    src/popen.c \
    src/ctype.c \
    $(SELECT_SRC) \
    src/qsort.c \
    src/getopt.c \
    src/dlfcn.c \
    src/getopt_long.c \
    src/getopt_long_only.c \
    src/locale.c \
    src/wchar.c \
    src/wchar_conv.c \
    src/wctype.c \
    src/tempfile.c \
    src/sysconf.c \
    src/syslog.c \
    src/getline.c \
    src/pwd.c \
    src/grp.c \
    src/uid.c \
    src/math.c \
    src/math_extra.c \
    src/rlimit.c \
    src/regex.c \
    src/fnmatch.c \
    src/scandir.c \
    src/ftw.c \
    src/glob.c \
    src/wprintf.c \
    src/wscanf.c

ARCH_SRC := $(wildcard src/arch/$(ARCH)/*.c)
SRC += $(if $(ARCH_SRC),$(ARCH_SRC),src/setjmp.c)

OBJ := $(SRC:.c=.o)
LIB := libvlibc.a
TEST_SRC := $(wildcard tests/*.c)
TEST_BIN := tests/run_tests
PLUGIN_SO := tests/plugin.so

all: $(LIB)

$(LIB): $(OBJ)
	$(AR) rcs $@ $^

$(PLUGIN_SO): tests/plugin.c
	$(CC) -shared -fPIC tests/plugin.c -o $(PLUGIN_SO)

$(TEST_BIN): $(TEST_SRC) $(LIB) $(PLUGIN_SO)
	$(CC) $(CFLAGS) $(TEST_SRC) $(LIB) -lpthread -o $@

test: $(TEST_BIN)
	./$(TEST_BIN)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


install: $(LIB)
	install -d $(DESTDIR)$(PREFIX)/lib
	install -m 644 $(LIB) $(DESTDIR)$(PREFIX)/lib
	install -d $(DESTDIR)$(PREFIX)/include
	install -m 644 include/*.h $(DESTDIR)$(PREFIX)/include
# ensure the new unistd.h header is installed
	install -m 644 include/unistd.h $(DESTDIR)$(PREFIX)/include
# install semaphore header
	install -m 644 include/semaphore.h $(DESTDIR)$(PREFIX)/include
	install -d $(DESTDIR)$(PREFIX)/include/sys
	install -m 644 include/sys/*.h $(DESTDIR)$(PREFIX)/include/sys

clean:
	rm -f $(OBJ) $(LIB) $(TEST_BIN) $(PLUGIN_SO)

.PHONY: all install clean test
