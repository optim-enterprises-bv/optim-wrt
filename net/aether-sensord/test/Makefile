# Host-side unit tests. No OpenWrt toolchain, no device, no nDPI.
CFLAGS ?= -std=gnu11 -Wall -Wextra -Werror -O1 -g -D_GNU_SOURCE
BINS = test_sigdb test_policy test_nft
.PHONY: all check clean
all: check
test_sigdb: test_sigdb.c ../src/sigdb.c ../src/match.c ../src/sigdb.h ../src/match.h
	$(CC) $(CFLAGS) -o $@ test_sigdb.c ../src/sigdb.c ../src/match.c
test_policy: test_policy.c ../src/policy.c ../src/sigdb.c ../src/policy.h ../src/sigdb.h
	$(CC) $(CFLAGS) -o $@ test_policy.c ../src/policy.c ../src/sigdb.c
test_nft: test_nft.c ../src/nft.c ../src/nft.h
	$(CC) $(CFLAGS) -o $@ test_nft.c ../src/nft.c

check: $(BINS)
	@for b in $(BINS); do echo "-- $$b"; ./$$b || exit 1; done
clean:
	rm -f $(BINS)
