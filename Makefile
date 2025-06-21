# Simple build rules for vlibc

PREFIX ?= /usr/local
CC ?= cc
CFLAGS ?= -O2 -std=c11 -Wall -Wextra -Iinclude
AR ?= ar

SRC := \
    src/errno.c \
    src/init.c \
    src/io.c \
    src/stdio.c \
    src/memory.c \
    src/process.c \
    src/string.c \
    src/socket.c \
    src/mmap.c \
    src/env.c \
    src/time.c \
    src/stat.c

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
