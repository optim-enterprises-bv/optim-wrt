# Host-side unit tests for the aggregation logic. No OpenWrt toolchain needed.
CFLAGS ?= -std=gnu11 -Wall -Wextra -Werror -O1 -g -D_GNU_SOURCE
.PHONY: all check clean
all: check
test_observe: test_observe.c ../src/observe.c ../src/observe.h
	$(CC) $(CFLAGS) -o $@ test_observe.c ../src/observe.c
check: test_observe
	./test_observe
clean:
	rm -f test_observe
