/*
 * Copyright (C) 2026 Optim Enterprises BV
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Compiles in the kernel and on the host. See af_match.h for why.
 */

#include "af_match.h"

/*
 * Bounds discipline used throughout.
 *
 * Every read goes through these. `p` is the cursor, `end` is one past the last
 * valid byte, and NOTHING advances the cursor without first proving the read
 * fits. There are no raw dereferences below this line.
 */
#define AF_HAVE(p, end, n) ((__u32)((end) - (p)) >= (__u32)(n))

static inline __u16 af_be16(const __u8 *p)
{
	return (__u16)(((__u16)p[0] << 8) | p[1]);
}

static inline __u32 af_be24(const __u8 *p)
{
	return ((__u32)p[0] << 16) | ((__u32)p[1] << 8) | p[2];
}

const char *af_extract_str(enum af_extract e)
{
	switch (e) {
	case AF_EXTRACT_OK:
		return "ok";
	case AF_EXTRACT_NOT_TLS:
		return "not_tls";
	case AF_EXTRACT_NOT_HELLO:
		return "not_client_hello";
	case AF_EXTRACT_TRUNCATED:
		return "truncated";
	case AF_EXTRACT_NO_SNI:
		return "no_sni";
	case AF_EXTRACT_NAME_TOO_LONG:
		return "name_too_long";
	case AF_EXTRACT_MALFORMED:
	default:
		return "malformed";
	}
}

static inline char af_lower(char c)
{
	return (c >= 'A' && c <= 'Z') ? (char)(c - 'A' + 'a') : c;
}

__u64 af_hash_name(const char *name, __u32 len)
{
	/* FNV-1a, 64-bit. Fixed constants so the daemon and the kernel cannot
	 * drift -- a divergence here silently stops every rule matching. */
	__u64 h = 1469598103934665603ULL;
	__u32 i;

	if (!name)
		return 0;
	for (i = 0; i < len && name[i]; i++) {
		h ^= (__u8)af_lower(name[i]);
		h *= 1099511628211ULL;
	}
	return h;
}

/*
 * Is this a plausible DNS name? Rejecting junk early keeps obvious garbage out
 * of the hash table and out of any log that later shows it to a human.
 */
static AF_BOOL af_name_plausible(const __u8 *p, __u32 n)
{
	__u32 i;
	__u32 dots = 0;

	if (n == 0 || n > AF_MAX_NAME_LEN)
		return false;
	if (p[0] == '.' || p[n - 1] == '.')
		return false;
	for (i = 0; i < n; i++) {
		__u8 c = p[i];
		AF_BOOL ok = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
		             (c >= '0' && c <= '9') || c == '.' || c == '-' ||
		             c == '_';
		if (!ok)
			return false;
		if (c == '.') {
			dots++;
			if (i > 0 && p[i - 1] == '.')
				return false; /* empty label */
		}
	}
	return dots > 0;
}

static void af_store_lower(const __u8 *src, __u32 n, char *out)
{
	__u32 i;
	for (i = 0; i < n; i++)
		out[i] = af_lower((char)src[i]);
	out[n] = '\0';
}

enum af_extract af_extract_sni(const __u8 *data, __u32 len, char *out,
                               __u32 out_len)
{
	const __u8 *p = data;
	const __u8 *end;
	__u32 hs_len, sess_len, cs_len, ext_total, comp_len;
	const __u8 *ext_end;

	if (!out || out_len <= AF_MAX_NAME_LEN)
		return AF_EXTRACT_MALFORMED;
	out[0] = '\0';
	if (!data || len == 0)
		return AF_EXTRACT_NOT_TLS;

	end = data + len;

	/* TLS record: type(1) version(2) length(2) */
	if (!AF_HAVE(p, end, 5))
		return AF_EXTRACT_TRUNCATED;
	if (p[0] != 0x16) /* handshake */
		return AF_EXTRACT_NOT_TLS;
	if (p[1] != 0x03) /* SSL 3.x / TLS 1.x major */
		return AF_EXTRACT_NOT_TLS;
	p += 5;

	/* Handshake: type(1) length(3) */
	if (!AF_HAVE(p, end, 4))
		return AF_EXTRACT_TRUNCATED;
	if (p[0] != 0x01) /* client_hello */
		return AF_EXTRACT_NOT_HELLO;
	hs_len = af_be24(p + 1);
	p += 4;
	/* Trust the SMALLER of the declared length and the real buffer. A
	 * ClientHello claiming to be longer than the packet is the classic way
	 * to walk a parser off the end, so a declared length is never allowed
	 * to extend `end` -- only to shrink it. */
	{
		__u32 remaining = (__u32)(end - p);
		if (hs_len < remaining)
			end = p + hs_len;
	}

	/* client_version(2) random(32) */
	if (!AF_HAVE(p, end, 34))
		return AF_EXTRACT_TRUNCATED;
	p += 34;

	/* session_id */
	if (!AF_HAVE(p, end, 1))
		return AF_EXTRACT_TRUNCATED;
	sess_len = *p++;
	if (!AF_HAVE(p, end, sess_len))
		return AF_EXTRACT_TRUNCATED;
	p += sess_len;

	/* cipher_suites */
	if (!AF_HAVE(p, end, 2))
		return AF_EXTRACT_TRUNCATED;
	cs_len = af_be16(p);
	p += 2;
	if (!AF_HAVE(p, end, cs_len))
		return AF_EXTRACT_TRUNCATED;
	p += cs_len;

	/* compression_methods */
	if (!AF_HAVE(p, end, 1))
		return AF_EXTRACT_TRUNCATED;
	comp_len = *p++;
	if (!AF_HAVE(p, end, comp_len))
		return AF_EXTRACT_TRUNCATED;
	p += comp_len;

	/* extensions */
	if (!AF_HAVE(p, end, 2))
		return AF_EXTRACT_NO_SNI; /* legal: a hello may carry none */
	ext_total = af_be16(p);
	p += 2;
	if (!AF_HAVE(p, end, ext_total))
		return AF_EXTRACT_TRUNCATED;
	ext_end = p + ext_total;

	/* Bounded by ext_end, which is bounded by the buffer. Cannot spin. */
	while (p + 4 <= ext_end) {
		__u16 etype = af_be16(p);
		__u16 elen = af_be16(p + 2);
		const __u8 *ebody = p + 4;

		if (ebody + elen > ext_end)
			return AF_EXTRACT_MALFORMED;

		if (etype == 0x0000) { /* server_name */
			const __u8 *q = ebody;
			const __u8 *qend = ebody + elen;
			__u16 list_len;

			if (q + 2 > qend)
				return AF_EXTRACT_MALFORMED;
			list_len = af_be16(q);
			q += 2;
			if (q + list_len > qend)
				return AF_EXTRACT_MALFORMED;
			qend = q + list_len;

			while (q + 3 <= qend) {
				__u8 ntype = q[0];
				__u16 nlen = af_be16(q + 1);
				const __u8 *nptr = q + 3;

				if (nptr + nlen > qend)
					return AF_EXTRACT_MALFORMED;
				if (ntype == 0x00) { /* host_name */
					if (nlen > AF_MAX_NAME_LEN)
						return AF_EXTRACT_NAME_TOO_LONG;
					if (!af_name_plausible(nptr, nlen))
						return AF_EXTRACT_MALFORMED;
					af_store_lower(nptr, nlen, out);
					return AF_EXTRACT_OK;
				}
				q = nptr + nlen;
			}
			return AF_EXTRACT_NO_SNI;
		}
		p = ebody + elen;
	}

	return AF_EXTRACT_NO_SNI;
}

enum af_extract af_extract_http_host(const __u8 *data, __u32 len, char *out,
                                     __u32 out_len)
{
	__u32 scan, i;
	static const char HOST[] = "host:";

	if (!out || out_len <= AF_MAX_NAME_LEN)
		return AF_EXTRACT_MALFORMED;
	out[0] = '\0';
	if (!data || len == 0)
		return AF_EXTRACT_NOT_TLS;

	/* Bounded: never scan a whole body looking for a header. */
	scan = (len < AF_HTTP_SCAN_MAX) ? len : AF_HTTP_SCAN_MAX;

	for (i = 0; i + 5 < scan; i++) {
		__u32 j;
		AF_BOOL hit = true;
		for (j = 0; j < 5; j++) {
			if (af_lower((char)data[i + j]) != HOST[j]) {
				hit = false;
				break;
			}
		}
		if (!hit)
			continue;
		/* Must be at a line start, or it is a substring of something
		 * else (e.g. "X-Forwarded-Host:"). */
		if (i != 0 && data[i - 1] != '\n')
			continue;

		{
			__u32 v = i + 5;
			__u32 nstart, nlen = 0;
			while (v < scan && (data[v] == ' ' || data[v] == '\t'))
				v++;
			nstart = v;
			while (v < scan && data[v] != '\r' && data[v] != '\n' &&
			       data[v] != ':')
				v++;
			nlen = v - nstart;
			if (nlen == 0)
				return AF_EXTRACT_MALFORMED;
			if (nlen > AF_MAX_NAME_LEN)
				return AF_EXTRACT_NAME_TOO_LONG;
			if (!af_name_plausible(data + nstart, nlen))
				return AF_EXTRACT_MALFORMED;
			af_store_lower(data + nstart, nlen, out);
			return AF_EXTRACT_OK;
		}
	}
	return AF_EXTRACT_NO_SNI;
}

/* ------------------------------------------------------------ hashset --- */

/* 0 marks an empty slot, so a genuine hash of 0 is remapped to this. */
#define AF_HASH_ZERO_SUBST 0xFFFFFFFFFFFFFFFFULL

AF_BOOL af_hashset_init(struct af_hashset *hs, __u64 *slots, __u32 capacity)
{
	if (!hs || !slots || capacity == 0)
		return false;
	/* Power of two, so the modulo is a mask and the probe cannot skip. */
	if ((capacity & (capacity - 1)) != 0)
		return false;
	hs->slots = slots;
	hs->capacity = capacity;
	hs->count = 0;
	memset(slots, 0, (size_t)capacity * sizeof(__u64));
	return true;
}

void af_hashset_clear(struct af_hashset *hs)
{
	if (!hs || !hs->slots)
		return;
	memset(hs->slots, 0, (size_t)hs->capacity * sizeof(__u64));
	hs->count = 0;
}

AF_BOOL af_hashset_insert(struct af_hashset *hs, __u64 hash)
{
	__u32 mask, idx, probe;

	if (!hs || !hs->slots)
		return false;
	if (hash == 0)
		hash = AF_HASH_ZERO_SUBST;
	/* Refuse at capacity rather than probe forever. This is the bound the
	 * reference implementation did not have. */
	if (hs->count >= hs->capacity)
		return false;

	mask = hs->capacity - 1;
	idx = (__u32)(hash & mask);
	for (probe = 0; probe < hs->capacity; probe++) {
		__u32 s = (idx + probe) & mask;
		if (hs->slots[s] == hash)
			return true; /* already present */
		if (hs->slots[s] == 0) {
			hs->slots[s] = hash;
			hs->count++;
			return true;
		}
	}
	return false;
}

AF_BOOL af_hashset_contains(const struct af_hashset *hs, __u64 hash)
{
	__u32 mask, idx, probe;

	if (!hs || !hs->slots || hs->count == 0)
		return false;
	if (hash == 0)
		hash = AF_HASH_ZERO_SUBST;

	mask = hs->capacity - 1;
	idx = (__u32)(hash & mask);
	for (probe = 0; probe < hs->capacity; probe++) {
		__u32 s = (idx + probe) & mask;
		if (hs->slots[s] == hash)
			return true;
		if (hs->slots[s] == 0)
			return false; /* open addressing: a gap ends the chain */
	}
	return false;
}
