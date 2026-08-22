# Host-side unit tests. No OpenWrt toolchain, no device, no nDPI.
CFLAGS ?= -std=gnu11 -Wall -Wextra -Werror -O1 -g -D_GNU_SOURCE
.PHONY: all check clean
all: check
test_sigdb: test_sigdb.c ../src/sigdb.c ../src/match.c ../src/sigdb.h ../src/match.h
	$(CC) $(CFLAGS) -o $@ test_sigdb.c ../src/sigdb.c ../src/match.c
check: test_sigdb
	./test_sigdb
clean:
	rm -f test_sigdb
