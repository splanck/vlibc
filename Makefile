# Simple build rules for vlibc

PREFIX ?= /usr/local
CC ?= cc
CFLAGS ?= -O2 -Wall -Wextra -fno-stack-protector -fno-builtin -Iinclude
CFLAGS += -std=c11 -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0
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

       # Detect presence of system ucontext implementation
HAVE_SYS_UCONTEXT := $(shell printf '#include <ucontext.h>\nint main(void){ucontext_t uc; getcontext(&uc); makecontext(&uc,(void(*)(void))0,0); setcontext(&uc);return 0;}' | $(CC) -x c - -Werror -c -o /dev/null 2>/dev/null && echo 1 || echo 0)
ifeq ($(HAVE_SYS_UCONTEXT),1)
CFLAGS += -DVLIBC_HAS_SYS_UCONTEXT=1
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
    src/err.c \
    src/init.c \
    src/io.c \
    src/stdio.c \
    src/printf.c \
    src/scanf.c \
    src/memory.c \
    src/memory_ops.c \
    src/atexit.c \
    src/aio.c \
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
    src/rand48.c \
    src/abs.c \
    src/arc4random.c \
    src/socket.c \
    src/socketpair.c \
    src/setsockopt.c \
    src/socket_msg.c \
    src/getsockopt.c \
    src/netdb.c \
    src/inet_pton.c \
    src/inet_ntop.c \
    src/inet_addr.c \
    src/inet_aton.c \
    src/ifaddrs.c \
    src/fd.c \
    src/fcntl.c \
    src/ioctl.c \
    src/isatty.c \
    src/ttyname.c \
    src/termios.c \
    src/pty.c \
    src/file.c \
    src/file_perm.c \
    src/flock.c \
    src/fsync.c \
    src/sync.c \
    src/truncate.c \
    src/posix_fallocate.c \
    src/posix_fadvise.c \
    src/posix_madvise.c \
    src/dir.c \
    src/access.c \
    src/getcwd.c \
    src/realpath.c \
    src/chroot.c \
    src/path.c \
    src/pathconf.c \
    src/mkfifo.c \
    src/mknod.c \
    src/ftok.c \
    $(SYS_SRC) \
    src/mmap.c \
    src/msync.c \
    src/mlock.c \
    src/shm.c \
    src/sysv_msg.c \
    src/sysv_sem.c \
    src/sysv_shm.c \
    src/mqueue.c \
    src/env.c \
    src/hostname.c \
    src/uname.c \
    src/sleep.c \
    src/sched.c \
    src/clock_gettime.c \
    src/clock_getres.c \
    src/clock_settime.c \
    src/time.c \
    src/time_conv.c \
    src/timespec_get.c \
    src/time_r.c \
    src/times.c \
    src/itimer.c \
    src/timer.c \
    src/strftime.c \
    src/wcsftime.c \
    src/strptime.c \
    src/strfmon.c \
    src/stat.c \
    src/statvfs.c \
    src/utime.c \
    src/pthread.c \
    src/pthread_attr.c \
    src/pthread_rwlock.c \
    src/pthread_spin.c \
    src/pthread_barrier.c \
    src/semaphore.c \
    src/semaphore_named.c \
    src/dirent.c \
    src/default_shell.c \
    src/popen.c \
    src/ctype.c \
    $(SELECT_SRC) \
    src/qsort.c \
    src/search_hash.c \
    src/search_tree.c \
    src/getopt.c \
    src/dlfcn.c \
    src/getopt_long.c \
    src/getopt_long_only.c \
    src/getsubopt.c \
    src/locale.c \
    src/locale_extra.c \
    src/langinfo.c \
    src/wchar.c \
    src/wchar_conv.c \
    src/wchar_io.c \
    src/wctype.c \
    src/wmem.c \
    src/memstream.c \
    src/fopencookie.c \
    src/tempfile.c \
    src/confstr.c \
    src/sysconf.c \
    src/syslog.c \
    src/getline.c \
    src/getloadavg.c \
    src/getpass.c \
    src/crypt.c \
    src/getlogin.c \
    src/pwd_r.c \
    src/grp_r.c \
    src/pwd.c \
    src/grp.c \
    src/uid.c \
    src/math.c \
    src/math_extra.c \
    src/complex.c \
    src/fenv.c \
    src/rlimit.c \
    src/getrusage.c \
    src/regex.c \
    src/fnmatch.c \
    src/scandir.c \
    src/fts.c \
    src/ftw.c \
    src/glob.c \
    src/wordexp.c \
    src/vis.c \
    src/wprintf.c \
    src/wscanf.c \
    src/ucontext.c \
    src/sigsetjmp.c \
    src/fmtmsg.c \
    src/progname.c

ifeq ($(HAVE_SYS_UCONTEXT),1)
SRC := $(filter-out src/ucontext.c,$(SRC))
endif

ARCH_SRC := $(wildcard src/arch/$(ARCH)/*.c)
SRC += $(if $(ARCH_SRC),$(ARCH_SRC),src/setjmp.c)

OBJ := $(SRC:.c=.o)
LIB := libvlibc.a
TEST_SRC := $(wildcard tests/*.c)
TEST_BIN := tests/run_tests
PLUGIN_SO := tests/plugin.so
TEST_GROUP ?=

all: $(LIB)

$(LIB): $(OBJ)
	$(AR) rcs $@ $^

$(PLUGIN_SO): tests/plugin.c
	$(CC) -shared -fPIC tests/plugin.c -o $(PLUGIN_SO)

$(TEST_BIN): $(TEST_SRC) $(LIB) $(PLUGIN_SO)
	$(CC) $(CFLAGS) -no-pie $(TEST_SRC) $(LIB) -lpthread -lcrypto -lm -o $@
test: $(TEST_BIN)
test-memory: TEST_GROUP=memory

test-network: TEST_GROUP=network
test-network: test
# Run a single test. Usage: make test-name NAME=<case>
test-name: $(TEST_BIN)
	TEST_NAME=$(NAME) $(TEST_BIN)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

install: $(LIB)
	install -d $(DESTDIR)$(PREFIX)/lib
	install -m 644 $(LIB) $(DESTDIR)$(PREFIX)/lib
	install -d $(DESTDIR)$(PREFIX)/include
	install -m 644 include/*.h $(DESTDIR)$(PREFIX)/include
	# ensure the new unistd.h header is installed
	install -m 644 include/unistd.h $(DESTDIR)$(PREFIX)/include
	# ensure the new assert.h header is installed
	install -m 644 include/assert.h $(DESTDIR)$(PREFIX)/include
	# ensure iconv.h is installed
	install -m 644 include/iconv.h $(DESTDIR)$(PREFIX)/include
	# install sched.h
	install -m 644 include/sched.h $(DESTDIR)$(PREFIX)/include
	install -d $(DESTDIR)$(PREFIX)/include/sys
	# install system headers including wait.h
	install -m 644 include/sys/*.h $(DESTDIR)$(PREFIX)/include/sys

clean:
	rm -f $(OBJ) $(LIB) $(TEST_BIN) $(PLUGIN_SO)

.PHONY: all install clean test test-memory test-network test-name
