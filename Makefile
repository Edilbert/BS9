EXE = bs9
EXE2 = form9
PREFIX ?= /usr/local

ifeq ($(WIN),y)
# build 32 bit executable
    CC = i686-w64-mingw32-gcc
# build 64 bit executable
#   CC = x86_64-w64-mingw32-gcc
   EXE := $(EXE).exe
   EXE2 := $(EXE2).exe
endif


all: $(EXE) $(EXE2)

$(EXE):	bs9.c
	$(CC) -Wall -Wextra -pedantic -std=c99 $< -o $@ -g

$(EXE2): form9.c
	$(CC) -Wall -Wextra -pedantic -std=c99 $< -o $@ -g

install:
	install $(EXE) $(PREFIX)/bin
	install $(EXE2) $(PREFIX)/bin

clean:
	rm -f $(EXE) $(EXE2)

dist: $(EXE) $(EXE2)
	zip bs9.zip $(EXE) $(EXE2) README.md

.PHONY: all install clean
