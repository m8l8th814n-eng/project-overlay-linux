CC = gcc
CFLAGS = -O2 -Wall -Wno-implicit-function-declaration -Wno-format-truncation

all: toggle install

toggle: toggle.c
	$(CC) $(CFLAGS) -o toggle toggle.c

install: install.c
	$(CC) $(CFLAGS) -o install install.c

clean:
	rm -f toggle install

.PHONY: all clean
