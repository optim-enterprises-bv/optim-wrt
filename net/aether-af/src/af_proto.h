/*
 * Copyright (C) 2026 Optim Enterprises BV
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * aether-af -- netlink protocol between the kernel module and aether-sensord.
 *
 * THIS FILE DEFINES THE LICENCE BOUNDARY, so it is worth stating plainly what
 * is on each side.
 *
 *   Kernel side (GPL-2.0): a mechanism. It extracts a TLS SNI / HTTP Host from
 *   a packet, hashes it, looks the hash up in a table, and drops or accepts.
 *   It contains NO signatures, NO application names, NO categories, NO policy
 *   and NO taxonomy. It cannot tell you what YouTube is.
 *
 *   Userspace side (BSD-3-Clause): the product. The 1,347-signature database,
 *   the matching semantics, the tag vocabulary, per-subject policy, schedules,
 *   quotas and everything the controller reasons about. It compiles all of
 *   that down to (hash, subject, verdict) triples and pushes them here.
 *
 * The two are separate programs exchanging data over an arm's-length netlink
 * interface. That is what keeps the GPL obligation confined to the mechanism
 * and off the product. Moving signature matching INTO the module would move
 * the product across the boundary with it, which is precisely the mistake
 * this design exists to avoid.
 *
 * Included by kernel code and by userspace, so it must stay free of anything
 * that only exists on one side.
 */

#ifndef AETHER_AF_PROTO_H
#define AETHER_AF_PROTO_H

#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <stdint.h>
typedef uint8_t __u8;
typedef uint16_t __u16;
typedef uint32_t __u32;
typedef uint64_t __u64;
#endif

/*
 * Netlink unit id. 29 is what Open App Filter used; this is deliberately NOT
 * 29, because both modules must be able to exist on one system during any
 * transition, and silently binding the same unit would make the second
 * netlink_kernel_create fail in a way that reads as "module broken".
 */
#define AF_NETLINK_UNIT 31

/* Bumped on any incompatible change. The module refuses a mismatch rather
 * than misparsing a message from a newer daemon -- a wrong rule table is
 * worse than no rule table, because it looks like it is working. */
#define AF_PROTO_VERSION 1

/* Message types, daemon -> kernel. */
enum af_msg_type {
	AF_MSG_NOOP = 0,
	AF_MSG_HELLO,        /* version handshake, must be first */
	AF_MSG_RULES_BEGIN,  /* start a replacement rule set */
	AF_MSG_RULE_ADD,     /* one (subject, hash) rule */
	AF_MSG_RULES_COMMIT, /* swap the staged set in atomically */
	AF_MSG_SUBJECT_ADD,  /* enrol a MAC as a policy subject */
	AF_MSG_SUBJECT_CLEAR,
	AF_MSG_STATS_REQ, /* ask for counters */
	AF_MSG_MAX
};

/* Message types, kernel -> daemon. */
enum af_evt_type {
	AF_EVT_NONE = 0,
	AF_EVT_STATS, /* counter snapshot */
	AF_EVT_ERROR,
	AF_EVT_MAX
};

#define AF_MAC_LEN 6

/*
 * A rule: this subject must not reach anything whose name hashes to this.
 *
 * `name_hash` is FNV-1a over the lowercased name, computed identically on both
 * sides by af_hash_name() in af_match.h. The kernel never sees the name it
 * came from, which is the point -- the signature database does not cross the
 * boundary, only its fingerprints do.
 *
 * A hash collision would over-block one unrelated host. With 64-bit FNV-1a
 * over ~1,347 names that is vanishingly unlikely, and the daemon can detect
 * collisions at compile time because it holds both sides. The kernel cannot,
 * and must not try.
 */
struct af_rule {
	__u64 name_hash;
	__u8 mac[AF_MAC_LEN];
	/* 0 = this rule applies to every enrolled subject. Non-zero = only the
	 * MAC above. Household-wide by accident is a failure mode ADR-017
	 * names explicitly, so it must be stated per rule, never inferred. */
	__u8 per_subject;
	__u8 _pad;
} __attribute__((packed));

struct af_subject {
	__u8 mac[AF_MAC_LEN];
	__u8 _pad[2];
} __attribute__((packed));

struct af_hello {
	__u32 version;
	__u32 _pad;
} __attribute__((packed));

struct af_stats {
	__u64 packets_seen;
	__u64 names_extracted;
	__u64 matched;
	__u64 dropped;
	__u64 rules_loaded;
	__u64 subjects_loaded;
	/* Rows refused because a table was full. Reported, never silently
	 * dropped -- a truncated rule set enforces less than the controller
	 * believes, and this is the counter that says so. */
	__u64 rules_refused;
	__u64 subjects_refused;
	/* Packets whose name could not be parsed. High values mean the
	 * extractor is being fed something it does not understand, which is
	 * useful long before it means an attack. */
	__u64 parse_failed;
} __attribute__((packed));

/* Every message begins with this. */
struct af_msg_hdr {
	__u16 type;    /* enum af_msg_type / af_evt_type */
	__u16 count;   /* payload records following */
	__u32 len;     /* payload bytes following this header */
} __attribute__((packed));

/*
 * Hard bounds. The kernel refuses anything above these and counts the refusal.
 *
 * Sized for what we ship with headroom, NOT for what someone else ships. The
 * reference implementation used a fixed 1024 with no bound check at all, which
 * corrupted memory from row 1025 and died around 1098 under traffic. In
 * userspace that cost a day of diagnosis; here it would be a panic on a
 * subscriber's router.
 */
#define AF_MAX_RULES 8192
#define AF_MAX_SUBJECTS 256
#define AF_MAX_MSG_LEN 8192

/* Longest name we will extract. Anything longer is refused, not truncated:
 * a truncated name hashes to something else entirely and would match the
 * wrong rule. */
#define AF_MAX_NAME_LEN 253

#endif /* AETHER_AF_PROTO_H */
