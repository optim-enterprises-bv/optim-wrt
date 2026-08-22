# Host tests for the kernel-module parsing core.
#
# This code runs in ring 0 on hostile input, so it is tested under ASan and
# UBSan as well as plain. A single byte of overread here is a panic on a
# subscriber's router; on the host it is a test failure.
CFLAGS ?= -std=gnu11 -Wall -Wextra -Werror -O1 -g
SAN = -fsanitize=address,undefined -fno-omit-frame-pointer
.PHONY: all check asan clean
all: check asan
test_match: test_match.c ../src/af_match.c ../src/af_match.h ../src/af_proto.h
	$(CC) $(CFLAGS) -o $@ test_match.c ../src/af_match.c
test_match_asan: test_match.c ../src/af_match.c
	$(CC) $(CFLAGS) $(SAN) -o $@ test_match.c ../src/af_match.c
check: test_match
	./test_match
	@echo
	@echo "NOTE: this run had NO sanitizer. The `asan` target is the one that"
	@echo "proves the absence of overreads, and it could not be built on the"
	@echo "host where this was developed (no libasan/libubsan runtime, no"
	@echo "valgrind). Run it somewhere that has them BEFORE this code is"
	@echo "compiled into a module -- a single byte of overread here is a panic"
	@echo "on a subscriber router, and plain -O1 will not catch it."
asan: test_match_asan
	@echo "-- under ASan + UBSan:"
	./test_match_asan
clean:
	rm -f test_match test_match_asan
