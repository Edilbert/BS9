EXE = bs9
PREFIX ?= /usr/local

ifeq ($(WIN),y)
# build 32 bit executable
    CC = i686-w64-mingw32-gcc
# build 64 bit executable
#   CC = x86_64-w64-mingw32-gcc
   EXE := $(EXE).exe
endif


all: $(EXE)

$(EXE):	bs9.c
	$(CC) -Wall -Wextra -pedantic -std=c99 $< -o $@ -g

install:
	install $(EXE) $(PREFIX)/bin
clean:
	rm -f $(EXE)

.PHONY: all install clean
