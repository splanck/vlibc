# Simple build rules for vlibc

PREFIX ?= /usr/local
CC ?= cc
CFLAGS ?= -O2 -std=c11 -Wall -Wextra -fno-stack-protector -fno-builtin -Iinclude
AR ?= ar

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
    src/system.c \
    src/string.c \
    src/strto.c \
    src/rand.c \
    src/socket.c \
    src/fd.c \
    src/file.c \
    src/dir.c \
    src/syscall.c \
    src/mmap.c \
    src/env.c \
    src/sleep.c \
    src/time.c \
    src/strftime.c \
    src/stat.c \
    src/pthread.c \
    src/dirent.c \
    src/popen.c \
    src/ctype.c \
    src/select.c \
    src/qsort.c \
    src/getopt.c \
    src/wchar.c

OBJ := $(SRC:.c=.o)
LIB := libvlibc.a
TEST_SRC := $(wildcard tests/*.c)
TEST_BIN := tests/run_tests

all: $(LIB)

$(LIB): $(OBJ)
	$(AR) rcs $@ $^

$(TEST_BIN): $(TEST_SRC) $(LIB)
	$(CC) $(CFLAGS) $(TEST_SRC) $(LIB) -o $@

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
	rm -f $(OBJ) $(LIB) $(TEST_BIN)

.PHONY: all install clean test
