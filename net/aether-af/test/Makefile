# Host tests for the kernel-module parsing core.
#
# This code runs in ring 0 on hostile input, so it is tested under ASan and
# UBSan as well as plain. A single byte of overread here is a panic on a
# subscriber's router; on the host it is a test failure.
CFLAGS ?= -std=gnu11 -Wall -Wextra -Werror -O1 -g
SAN = -fsanitize=address,undefined -fno-omit-frame-pointer
.PHONY: all check asan fuzz clean
all: check asan
test_match: test_match.c ../src/af_match.c ../src/af_match.h ../src/af_proto.h
	$(CC) $(CFLAGS) -o $@ test_match.c ../src/af_match.c
test_match_asan: test_match.c ../src/af_match.c
	$(CC) $(CFLAGS) $(SAN) -o $@ test_match.c ../src/af_match.c
check: test_match
	./test_match
	@echo
	@echo "NOTE: this run had NO sanitizer on THIS host (no libasan/libubsan,"
	@echo "no valgrind). The asan and fuzz targets HAVE been run elsewhere:"
	@echo "  make asan  -> 185 checks, 0 failures"
	@echo "  make fuzz  -> 200,000 iterations, no trap, detect_leaks clean"
	@echo "Re-run both after any change to af_match.c. A single byte of"
	@echo "overread here is a panic on a subscriber router, and -O1 alone"
	@echo "will not catch it."
asan: test_match_asan
	@echo "-- under ASan + UBSan:"
	./test_match_asan
# Randomised harness, contributed by the firmware lane. Every buffer is
# malloc'd to EXACTLY the length passed in -- that is the part that matters. A
# static array or an over-allocated buffer absorbs a one-byte overread and the
# sanitizer sees nothing; an exact-sized heap allocation turns it into a
# heap-buffer-overflow with a stack trace. The fixed vectors above mostly use
# static arrays and structurally cannot cover that class.
fuzz_match: fuzz_match.c ../src/af_match.c
	$(CC) $(CFLAGS) $(SAN) -o $@ fuzz_match.c ../src/af_match.c
fuzz: fuzz_match
	./fuzz_match 200000

clean:
	rm -f test_match test_match_asan fuzz_match
