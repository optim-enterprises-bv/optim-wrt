# Host-side unit tests for the aggregation logic. No OpenWrt toolchain needed.
CFLAGS ?= -std=gnu11 -Wall -Wextra -Werror -O1 -g -D_GNU_SOURCE
.PHONY: all check clean
all: check
test_observe: test_observe.c ../src/observe.c ../src/observe.h
	$(CC) $(CFLAGS) -o $@ test_observe.c ../src/observe.c
test_nflog_raw: test_nflog_raw.c ../src/nflog_raw.c ../src/nflog_raw.h
	$(CC) $(CFLAGS) -o $@ test_nflog_raw.c ../src/nflog_raw.c

check: test_observe test_nflog_raw
	./test_observe
	./test_nflog_raw
clean:
	rm -f test_observe test_nflog_raw
