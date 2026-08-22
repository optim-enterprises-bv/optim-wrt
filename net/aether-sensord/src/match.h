/*
 * Copyright (C) 2026 Optim Enterprises BV
 *
 * This is free software, licensed under the BSD 3-Clause License.
 *
 * Signature matching for aether-sensord.
 *
 * nDPI dissects; we classify (ADR-020 decision 3). This file is the "we
 * classify" half: given the primitives a dissector extracts -- an SNI or HTTP
 * Host, a transport and a destination port -- decide which application in our
 * database the flow belongs to.
 *
 * Keeping this separate from the dissector is what lets one database serve
 * every platform: the same matcher runs over nDPI's `tls.sni` on OpenWrt and
 * over walleye's published `tls.sni` on OpenSync.
 *
 * Pure and host-testable: no nDPI, no sockets, no netlink.
 */

#ifndef AETHER_SENSORD_MATCH_H
#define AETHER_SENSORD_MATCH_H

#include "sigdb.h"

#include <stdbool.h>
#include <stdint.h>

/* How a flow matched, most specific first. Reported so a verdict can be
 * explained -- "why was this blocked" must be answerable after the fact. */
enum match_kind {
	MATCH_NONE = 0,
	MATCH_PORT,      /* transport + port only, e.g. samba on 445 */
	MATCH_HOST_SUFFIX, /* pattern is a parent domain of the host */
	MATCH_HOST_EXACT
};

struct match_result {
	enum match_kind kind;
	const struct sig_app *app; /* NULL when kind == MATCH_NONE */
	uint16_t rule_index;
};

const char *match_kind_str(enum match_kind k);

/*
 * Classify a flow.
 *
 * `host` may be NULL or empty when the dissector recovered no name (plain TCP,
 * an unparsed QUIC flow, a fragment) -- port-only rules can still match.
 * `proto` is SIG_PROTO_TCP/UDP, `dport` 0 if unknown.
 *
 * Specificity wins: an exact host match beats a suffix match, which beats a
 * port-only match. Among equally specific matches the LONGER pattern wins, so
 * "mail.google.com" is not shadowed by "google.com". Ties resolve to the first
 * rule loaded, which makes the outcome deterministic for a given database.
 */
struct match_result match_flow(const struct sig_db *db, const char *host,
                               uint8_t proto, uint16_t dport);

/*
 * Does `pattern` match `host` as a domain?
 *
 * Exact, or `host` ends with "." + pattern. Deliberately NOT a substring
 * test: a bare substring makes "evil-github.com.attacker.net" match "github.com",
 * and label-boundary matching is the difference between a signature and a
 * guess.
 */
bool match_host_pattern(const char *host, const char *pattern,
                        enum match_kind *kind);

#endif /* AETHER_SENSORD_MATCH_H */
