/*
 * Copyright (C) 2026 Optim Enterprises BV
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Host tests for af_match -- the code that parses attacker-controlled bytes
 * and then runs in ring 0.
 *
 * The malformed and adversarial cases come FIRST, deliberately. A parser that
 * handles well-formed input is not the interesting property; surviving a
 * ClientHello built to walk it off the end of a buffer is. Every length field
 * in TLS is attacker-controlled, and the failure mode in kernel space is a
 * panic on a remote subscriber router with no console.
 *
 * Run under ASan/UBSan as well as plain:
 *     make -C net/aether-af/test check
 *     make -C net/aether-af/test asan
 *
 * !! THE ASAN RUN HAS NOT BEEN PERFORMED. The development host has no
 * libasan/libubsan runtime and no valgrind, so every figure below comes from
 * an unsanitised build. The truncation test allocates each cut length exactly,
 * which is the best proxy available without a sanitizer, but a single-byte
 * overread would very likely pass unnoticed.
 *
 * This is a GATE, not a footnote: run the asan target on a host that has the
 * runtime before this file is compiled into a kernel module.
 */

#include "../src/af_match.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;
static int checks;

#define CHECK(cond, msg)                                                       \
	do {                                                                   \
		checks++;                                                      \
		if (!(cond)) {                                                 \
			failures++;                                            \
			fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__,          \
			        __LINE__, (msg));                              \
		}                                                              \
	} while (0)

static char NAME[AF_MAX_NAME_LEN + 1];

/* Build a ClientHello carrying `sni`. Returns length written. */
static __u32 build_hello(__u8 *b, __u32 cap, const char *sni)
{
	__u32 n = (__u32)strlen(sni);
	__u32 sni_body = 2 + 1 + 2 + n; /* list_len + type + name_len + name */
	__u32 exts = 4 + sni_body;
	__u32 body = 2 + 32 + 1 + 2 + 2 + 2 + 2 + exts;
	__u32 total = 5 + 4 + body;
	__u8 *p = b;

	if (cap < total)
		return 0;

	*p++ = 0x16; *p++ = 0x03; *p++ = 0x01;
	*p++ = (__u8)((body + 4) >> 8); *p++ = (__u8)((body + 4) & 0xff);

	*p++ = 0x01;
	*p++ = (__u8)(body >> 16); *p++ = (__u8)(body >> 8); *p++ = (__u8)body;

	*p++ = 0x03; *p++ = 0x03;
	memset(p, 0xAB, 32); p += 32;
	*p++ = 0x00;                              /* session_id len */
	*p++ = 0x00; *p++ = 0x02; *p++ = 0xc0; *p++ = 0x2f; /* ciphers */
	*p++ = 0x01; *p++ = 0x00;                 /* compression */
	*p++ = (__u8)(exts >> 8); *p++ = (__u8)exts;

	*p++ = 0x00; *p++ = 0x00;                 /* ext type: server_name */
	*p++ = (__u8)(sni_body >> 8); *p++ = (__u8)sni_body;
	*p++ = (__u8)((n + 3) >> 8); *p++ = (__u8)(n + 3);
	*p++ = 0x00;                              /* host_name */
	*p++ = (__u8)(n >> 8); *p++ = (__u8)n;
	memcpy(p, sni, n); p += n;

	return (__u32)(p - b);
}

/* ------------------------------------------- hostile / malformed first --- */

static void test_truncation_at_every_offset(void)
{
	/* The blunt instrument that finds most parser bugs: build a valid
	 * ClientHello, then feed it truncated at EVERY length. None may crash,
	 * none may report OK, none may read past the buffer (ASan proves the
	 * last one). */
	__u8 buf[512];
	__u32 full = build_hello(buf, sizeof(buf), "www.youtube.com");
	CHECK(full > 0, "fixture built");

	int bad = 0;
	for (__u32 cut = 0; cut < full; cut++) {
		/* Copy into an exactly-sized allocation so ASan catches a
		 * single byte of overread, which a large static buffer would
		 * silently absorb. */
		__u8 *exact = malloc(cut ? cut : 1);
		if (cut)
			memcpy(exact, buf, cut);
		enum af_extract r = af_extract_sni(exact, cut, NAME, sizeof(NAME));
		if (r == AF_EXTRACT_OK)
			bad++;
		if (NAME[0] != '\0' && r != AF_EXTRACT_OK)
			bad++; /* must not leave a stale name behind */
		free(exact);
	}
	CHECK(bad == 0, "no truncation reports OK or leaves a stale name");
}

static void test_lying_length_fields(void)
{
	__u8 buf[512];
	__u32 full = build_hello(buf, sizeof(buf), "www.youtube.com");

	/* Handshake length claims far more than the buffer holds.
	 *
	 * The correct behaviour is to CLAMP and carry on, not to refuse. The
	 * declared length may only shrink the parse window, never extend it, so
	 * there is no overread -- and the data really is present, so extraction
	 * should still succeed.
	 *
	 * Refusing here would be worse than useless: it would hand an attacker
	 * a one-byte way to SUPPRESS classification, by lying about a length so
	 * the parser gives up and the flow sails through unclassified. An
	 * earlier version of this test asserted refusal and was wrong. */
	__u8 a[512];
	memcpy(a, buf, full);
	a[6] = 0xFF; a[7] = 0xFF; a[8] = 0xFF;
	CHECK(af_extract_sni(a, full, NAME, sizeof(NAME)) == AF_EXTRACT_OK,
	      "oversized handshake length is clamped, not obeyed, and not fatal");
	CHECK(strcmp(NAME, "www.youtube.com") == 0,
	      "and the name is still recovered -- no evasion via a lying length");

	/* session_id length claims to swallow the rest. */
	memcpy(a, buf, full);
	a[43] = 0xFF;
	CHECK(af_extract_sni(a, full, NAME, sizeof(NAME)) == AF_EXTRACT_TRUNCATED,
	      "oversized session_id refused");

	/* extensions_total larger than what follows. */
	memcpy(a, buf, full);
	a[49] = 0xFF; a[50] = 0xFF;
	CHECK(af_extract_sni(a, full, NAME, sizeof(NAME)) != AF_EXTRACT_OK,
	      "oversized extensions block refused");
}

static void test_name_longer_than_the_limit_is_refused(void)
{
	/* Refused, never truncated: a truncated name hashes to something else
	 * and would match the WRONG rule. */
	char big[600];
	memset(big, 'a', sizeof(big) - 1);
	big[sizeof(big) - 1] = '\0';
	big[100] = '.';

	__u8 buf[2048];
	__u32 n = build_hello(buf, sizeof(buf), big);
	if (n > 0) {
		enum af_extract r = af_extract_sni(buf, n, NAME, sizeof(NAME));
		CHECK(r == AF_EXTRACT_NAME_TOO_LONG || r == AF_EXTRACT_MALFORMED,
		      "over-long name refused, not truncated");
		CHECK(NAME[0] == '\0', "and nothing written");
	}
}

static void test_junk_and_hostile_names(void)
{
	/* A name is a DNS name. Anything else is refused before it can reach a
	 * hash table or a log a human will read. */
	const char *junk[] = { "no-dots-here", ".leading.dot", "trailing.dot.",
		               "double..dot", "has space.com", "semi;colon.com",
		               "back`tick.com", "nul\x01" "byte.com", NULL };
	__u8 buf[1024];
	for (int i = 0; junk[i]; i++) {
		__u32 n = build_hello(buf, sizeof(buf), junk[i]);
		if (n == 0)
			continue;
		enum af_extract r = af_extract_sni(buf, n, NAME, sizeof(NAME));
		CHECK(r != AF_EXTRACT_OK, junk[i]);
		CHECK(NAME[0] == '\0', "nothing written for junk");
	}
}

static void test_zero_length_and_null(void)
{
	CHECK(af_extract_sni(NULL, 0, NAME, sizeof(NAME)) != AF_EXTRACT_OK, "null");
	__u8 empty[1] = { 0 };
	CHECK(af_extract_sni(empty, 0, NAME, sizeof(NAME)) != AF_EXTRACT_OK, "zero len");
	/* An undersized output buffer must be refused, not overflowed. */
	char tiny[4];
	CHECK(af_extract_sni(empty, 1, tiny, sizeof(tiny)) == AF_EXTRACT_MALFORMED,
	      "undersized output buffer refused");
}

static void test_not_tls(void)
{
	__u8 http[] = "GET / HTTP/1.1\r\nHost: example.com\r\n\r\n";
	CHECK(af_extract_sni(http, sizeof(http) - 1, NAME, sizeof(NAME)) ==
	          AF_EXTRACT_NOT_TLS,
	      "HTTP is not TLS");
	__u8 alert[] = { 0x15, 0x03, 0x03, 0x00, 0x02, 0x01, 0x00 };
	CHECK(af_extract_sni(alert, sizeof(alert), NAME, sizeof(NAME)) ==
	          AF_EXTRACT_NOT_TLS,
	      "TLS alert is not a handshake");
}

/* ------------------------------------------------------ the happy path --- */

static void test_extracts_real_names(void)
{
	const char *names[] = { "www.youtube.com", "youtubei.googleapis.com",
		                "tiktokcdn.com", "api.openai.com", NULL };
	__u8 buf[1024];
	for (int i = 0; names[i]; i++) {
		__u32 n = build_hello(buf, sizeof(buf), names[i]);
		CHECK(af_extract_sni(buf, n, NAME, sizeof(NAME)) == AF_EXTRACT_OK,
		      names[i]);
		CHECK(strcmp(NAME, names[i]) == 0, "exact name recovered");
	}
}

static void test_case_is_folded(void)
{
	__u8 buf[1024];
	__u32 n = build_hello(buf, sizeof(buf), "WWW.YouTube.COM");
	CHECK(af_extract_sni(buf, n, NAME, sizeof(NAME)) == AF_EXTRACT_OK, "ok");
	CHECK(strcmp(NAME, "www.youtube.com") == 0, "lowercased");
}

static void test_http_host(void)
{
	__u8 req[] = "GET /x HTTP/1.1\r\nUser-Agent: t\r\nHost: WWW.Example.COM\r\n\r\n";
	CHECK(af_extract_http_host(req, sizeof(req) - 1, NAME, sizeof(NAME)) ==
	          AF_EXTRACT_OK,
	      "host extracted");
	CHECK(strcmp(NAME, "www.example.com") == 0, "lowercased");

	/* Must not match a header that merely ends in "host:". */
	__u8 fwd[] = "GET / HTTP/1.1\r\nX-Forwarded-Host: evil.com\r\n\r\n";
	CHECK(af_extract_http_host(fwd, sizeof(fwd) - 1, NAME, sizeof(NAME)) !=
	          AF_EXTRACT_OK,
	      "X-Forwarded-Host is not Host");
}

/* --------------------------------------------------------- hash + set --- */

static void test_hash_is_case_insensitive_and_stable(void)
{
	__u64 a = af_hash_name("www.youtube.com", 15);
	__u64 b = af_hash_name("WWW.YouTube.COM", 15);
	CHECK(a == b, "hash folds case");
	CHECK(a != af_hash_name("www.youtube.co", 14), "different names differ");
	CHECK(af_hash_name(NULL, 5) == 0, "null is zero, not a crash");
}

static void test_hashset_bounds(void)
{
	enum { CAP = 64 };
	__u64 slots[CAP];
	struct af_hashset hs;
	CHECK(af_hashset_init(&hs, slots, CAP), "init");
	CHECK(!af_hashset_init(&hs, slots, 63), "non-power-of-two refused");

	for (int i = 0; i < CAP; i++)
		CHECK(af_hashset_insert(&hs, 1000 + i), "insert within capacity");
	CHECK(hs.count == CAP, "full");
	/* The bound that the reference implementation did not have. */
	CHECK(!af_hashset_insert(&hs, 99999), "insert past capacity REFUSED");
	CHECK(hs.count == CAP, "count unchanged by the refusal");

	for (int i = 0; i < CAP; i++)
		CHECK(af_hashset_contains(&hs, 1000 + i), "lookup finds it");
	CHECK(!af_hashset_contains(&hs, 99999), "refused entry is absent");
}

static void test_hashset_zero_hash(void)
{
	/* 0 marks an empty slot, so a genuine hash of 0 must be remapped or it
	 * would be invisible. */
	__u64 slots[16];
	struct af_hashset hs;
	af_hashset_init(&hs, slots, 16);
	CHECK(af_hashset_insert(&hs, 0), "hash 0 inserts");
	CHECK(af_hashset_contains(&hs, 0), "and is found");
}

static void test_hashset_duplicates(void)
{
	__u64 slots[16];
	struct af_hashset hs;
	af_hashset_init(&hs, slots, 16);
	af_hashset_insert(&hs, 42);
	af_hashset_insert(&hs, 42);
	CHECK(hs.count == 1, "duplicate does not consume a second slot");
}

static void test_end_to_end(void)
{
	/* What the module will actually do: extract, hash, look up. */
	__u64 slots[256];
	struct af_hashset hs;
	af_hashset_init(&hs, slots, 256);
	af_hashset_insert(&hs, af_hash_name("www.youtube.com", 15));

	__u8 buf[1024];
	__u32 n = build_hello(buf, sizeof(buf), "www.youtube.com");
	CHECK(af_extract_sni(buf, n, NAME, sizeof(NAME)) == AF_EXTRACT_OK, "extract");
	CHECK(af_hashset_contains(&hs, af_hash_name(NAME, (__u32)strlen(NAME))),
	      "blocked name matches");

	n = build_hello(buf, sizeof(buf), "www.wikipedia.org");
	af_extract_sni(buf, n, NAME, sizeof(NAME));
	CHECK(!af_hashset_contains(&hs, af_hash_name(NAME, (__u32)strlen(NAME))),
	      "unrelated name does not match");
}

int main(void)
{
	test_truncation_at_every_offset();
	test_lying_length_fields();
	test_name_longer_than_the_limit_is_refused();
	test_junk_and_hostile_names();
	test_zero_length_and_null();
	test_not_tls();
	test_extracts_real_names();
	test_case_is_folded();
	test_http_host();
	test_hash_is_case_insensitive_and_stable();
	test_hashset_bounds();
	test_hashset_zero_hash();
	test_hashset_duplicates();
	test_end_to_end();

	printf("%d checks, %d failures\n", checks, failures);
	return failures == 0 ? 0 : 1;
}
