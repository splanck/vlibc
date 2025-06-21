# Simple build rules for vlibc

PREFIX ?= /usr/local
CC ?= cc
CFLAGS ?= -O2 -std=c11 -Wall -Wextra -Iinclude
AR ?= ar

SRC := $(wildcard src/*.c)
OBJ := $(SRC:.c=.o)
LIB := libvlibc.a

all: $(LIB)

$(LIB): $(OBJ)
	$(AR) rcs $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

install: $(LIB)
	install -d $(DESTDIR)$(PREFIX)/lib
	install -m 644 $(LIB) $(DESTDIR)$(PREFIX)/lib
	install -d $(DESTDIR)$(PREFIX)/include
	install -m 644 include/*.h $(DESTDIR)$(PREFIX)/include

clean:
	rm -f $(OBJ) $(LIB)

.PHONY: all install clean
