/*
 * Copyright (C) 2026 Optim Enterprises BV
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Name extraction and hash lookup for aether-af.
 *
 * THIS FILE COMPILES BOTH IN THE KERNEL AND IN USERSPACE, on purpose.
 *
 * It parses attacker-controlled bytes -- a TLS ClientHello arriving from the
 * internet -- and it runs in ring 0. A mistake here is not a wrong verdict in
 * a log; it is a panic on a subscriber's router, remote, with no console, on
 * the box that is their only internet connection. Historically this codebase's
 * worst defect was a fixed 1024-entry array with no bound check that corrupted
 * memory from row 1025 and died ~70 rows later under traffic. That was in
 * USERSPACE and it cost a day to find.
 *
 * So the parsing is written once, free of kernel headers, and unit-tested on
 * the host against malformed, truncated, and adversarial input BEFORE it is
 * ever compiled into a module. The kernel side is glue around a function that
 * has already been proven not to walk off the end of a buffer.
 *
 * Every accessor is bounds-checked against an explicit end pointer. There are
 * no unbounded loops. There is no allocation. It cannot fail into a state
 * where the caller must guess.
 */

#ifndef AETHER_AF_MATCH_H
#define AETHER_AF_MATCH_H

#include "af_proto.h"

#ifdef __KERNEL__
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/types.h>
#define AF_BOOL bool
#else
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#define AF_BOOL bool
#endif

/* Why a name could not be extracted. Diagnostic only -- the caller treats
 * every non-OK result identically (no name, no match, accept). Reported so a
 * high parse-failure rate is visible long before it means an attack. */
enum af_extract {
	AF_EXTRACT_OK = 0,
	AF_EXTRACT_NOT_TLS,     /* not a TLS record we understand */
	AF_EXTRACT_NOT_HELLO,   /* TLS, but not a ClientHello */
	AF_EXTRACT_TRUNCATED,   /* ran out of buffer mid-structure */
	AF_EXTRACT_NO_SNI,      /* well-formed ClientHello, no SNI extension */
	AF_EXTRACT_NAME_TOO_LONG, /* refused, never truncated */
	AF_EXTRACT_MALFORMED    /* internal length fields disagree */
};

const char *af_extract_str(enum af_extract e);

/*
 * FNV-1a over the lowercased name.
 *
 * Must produce identical output in the kernel and in the daemon, because the
 * daemon compiles signatures to hashes and the kernel looks them up. Any
 * divergence silently stops every rule matching, which looks exactly like
 * "no traffic to block".
 */
__u64 af_hash_name(const char *name, __u32 len);

/*
 * Extract the SNI from a TLS ClientHello.
 *
 * `data`/`len` is the TCP payload. `out` receives a NUL-terminated lowercase
 * name of at most AF_MAX_NAME_LEN bytes; `out_len` must be >= AF_MAX_NAME_LEN+1.
 *
 * Returns AF_EXTRACT_OK only when a name was written. On every other result
 * `out` is set to an empty string, so a caller that ignores the return value
 * still cannot read a stale buffer.
 *
 * Refuses rather than truncates: a truncated name hashes to something else and
 * would match the wrong rule, which is worse than not matching at all.
 */
enum af_extract af_extract_sni(const __u8 *data, __u32 len, char *out,
                               __u32 out_len);

/*
 * Extract the Host header from an HTTP request. Same contract as above.
 *
 * Only the first line-terminated Host: header is considered, and only within
 * the first AF_HTTP_SCAN_MAX bytes -- an unbounded scan over a large body is
 * a denial-of-service against our own hook.
 */
#define AF_HTTP_SCAN_MAX 1024
enum af_extract af_extract_http_host(const __u8 *data, __u32 len, char *out,
                                     __u32 out_len);

/*
 * A fixed-capacity open-addressed hash set of blocked name-hashes.
 *
 * Sized at build time, never grown, never allocated inside the packet path.
 * Insert refuses at capacity and the refusal is counted by the caller -- the
 * table cannot be overrun, which is the single defect this whole file is
 * written in reaction to.
 */
struct af_hashset {
	__u64 *slots;    /* 0 means empty; a real hash of 0 is remapped */
	__u32 capacity;  /* power of two */
	__u32 count;
};

/* `capacity` must be a power of two and `slots` must have that many entries. */
AF_BOOL af_hashset_init(struct af_hashset *hs, __u64 *slots, __u32 capacity);
void af_hashset_clear(struct af_hashset *hs);
/* false when full. Never writes past the end. */
AF_BOOL af_hashset_insert(struct af_hashset *hs, __u64 hash);
AF_BOOL af_hashset_contains(const struct af_hashset *hs, __u64 hash);

#endif /* AETHER_AF_MATCH_H */
