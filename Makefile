# Simple build rules for vlibc

PREFIX ?= /usr/local
CC ?= cc
CFLAGS ?= -O2 -std=c11 -Wall -Wextra -fno-stack-protector -fno-builtin -Iinclude
AR ?= ar
ARCH ?= $(shell uname -m)

TARGET_OS ?= $(OS)
ifeq ($(TARGET_OS),)
TARGET_OS := $(shell uname -s)
endif

ifneq (,$(filter $(TARGET_OS),FreeBSD NetBSD OpenBSD DragonFly))
CFLAGS += -D__BSD_VISIBLE
endif

SYS_SRC := src/syscall.c
ifeq ($(TARGET_OS),Windows_NT)
SYS_SRC := src/arch/win32/syscall.c
endif

SRC := \
    src/errno.c \
    src/error.c \
    src/init.c \
    src/io.c \
    src/stdio.c \
    src/printf.c \
    src/memory.c \
    src/memory_ops.c \
    src/atexit.c \
    src/process.c \
    src/abort.c \
    src/system.c \
    src/string.c \
    src/string_extra.c \
    src/strto.c \
    src/rand.c \
    src/socket.c \
    src/netdb.c \
    src/fd.c \
    src/file.c \
    src/file_perm.c \
    src/dir.c \
    $(SYS_SRC) \
    src/mmap.c \
    src/env.c \
    src/sleep.c \
    src/time.c \
    src/time_conv.c \
    src/strftime.c \
    src/stat.c \
    src/pthread.c \
    src/dirent.c \
    src/default_shell.c \
    src/popen.c \
    src/ctype.c \
    src/select.c \
    src/qsort.c \
    src/getopt.c \
    src/dlfcn.c \
    src/getopt_long.c \
    src/locale.c \
    src/wchar.c \
    src/math.c

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
	install -d $(DESTDIR)$(PREFIX)/include/sys
	install -m 644 include/sys/*.h $(DESTDIR)$(PREFIX)/include/sys

clean:
	rm -f $(OBJ) $(LIB) $(TEST_BIN) $(PLUGIN_SO)

.PHONY: all install clean test
