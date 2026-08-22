// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2026 Optim Enterprises BV
 *
 * aether-af -- application filtering mechanism.
 *
 * WHAT THIS MODULE IS: a mechanism. It takes packets from a netfilter hook,
 * asks af_match.c for a name, hashes it, looks the hash up in a table it was
 * given, and drops or accepts. That is the whole of it.
 *
 * WHAT THIS MODULE IS NOT: the product. It holds no signatures, no application
 * names, no categories, no schedules, no quotas and no taxonomy. It cannot
 * tell you what YouTube is. Every decision it enforces was made in userspace
 * by aether-sensord (BSD-3-Clause) and compiled down to (hash, subject,
 * verdict) before it crossed the netlink boundary.
 *
 * That separation is deliberate and it is what confines this file's GPL
 * obligation to a few hundred lines of glue. Moving signature matching in here
 * would drag the product across the boundary with it -- which is exactly what
 * Open App Filter does, and exactly why OAF's product logic is GPL.
 *
 * DELIBERATE OMISSION: no TCP RST. nf_send_reset() is the one EXPORT_SYMBOL_GPL
 * symbol OAF depends on, and we block by silent drop instead. A blocked
 * connection hangs until timeout rather than failing fast; that is the price,
 * and it is a small one.
 *
 * The parsing this calls into is host-tested under adversarial input before it
 * ever runs here. See af_match.h for why that matters more than usual.
 *
 * SYMBOL LICENCE POSITION, measured rather than assumed. Every undefined symbol
 * in the built module was checked against Module.symvers for 6.18.44/filogic.
 * The result was two GPL-only symbols, and neither was the one anyone expected:
 * not nf_send_reset (unused here), not conntrack (unused), but call_rcu and
 * synchronize_rcu. Both were an implementation choice, not a requirement, and
 * both are replaced by synchronize_net() which is plain EXPORT_SYMBOL.
 *
 * This module now references ZERO EXPORT_SYMBOL_GPL symbols.
 *
 * That removes the TECHNICAL barrier to a non-GPL declaration. It does not
 * settle the LEGAL one: a module compiled against kernel headers and linked
 * into the kernel's namespace is widely held to be a derivative work
 * regardless of which symbols it touches, and MODULE_LICENSE is an assertion
 * by the author that nothing adjudicates. The declaration below stays GPL
 * until someone qualified says otherwise. What has changed is that the choice
 * is now a legal question alone, with no technical obstacle behind it.
 */

#include "af_match.h"
#include "af_proto.h"

#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv6.h>
#include <linux/netlink.h>
#include <linux/rculist.h>
#include <linux/skbuff.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/tcp.h>
#include <linux/version.h>
#include <net/netlink.h>
#include <net/sock.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Optim Enterprises BV");
MODULE_DESCRIPTION("aether application filtering mechanism (no policy, no signatures)");
MODULE_VERSION("0.1.0");

/* Ports we bother inspecting. Everything else is accepted without parsing --
 * an unbounded attempt to find a name in arbitrary traffic is a denial of
 * service against our own hook. */
#define AF_PORT_HTTPS 443
#define AF_PORT_HTTP 80

/*
 * The active rule set.
 *
 * Swapped atomically under RCU: the daemon stages a complete replacement and
 * commits it, so the packet path never sees a half-updated table. A partially
 * applied rule set is the failure that looks like it is working.
 */
struct af_ruleset {
	struct af_hashset any;   /* rules that apply to every subject */
	__u64 *any_slots;
	struct af_hashset per;   /* rules scoped to a subject */
	__u64 *per_slots;
	/* Subject MACs, linear -- there are at most AF_MAX_SUBJECTS and the
	 * lookup is off the hot path for unmatched traffic. */
	__u8 macs[AF_MAX_SUBJECTS][AF_MAC_LEN];
	__u32 n_macs;
};

static struct af_ruleset __rcu *af_active;
static struct af_ruleset *af_staging;
static DEFINE_MUTEX(af_stage_lock);

static struct sock *af_nl_sock;
static struct af_stats af_stats;
static DEFINE_SPINLOCK(af_stats_lock);

#define AF_STAT_INC(field)                                                     \
	do {                                                                   \
		unsigned long _f;                                              \
		spin_lock_irqsave(&af_stats_lock, _f);                         \
		af_stats.field++;                                              \
		spin_unlock_irqrestore(&af_stats_lock, _f);                    \
	} while (0)

/* Hash table capacity must be a power of two and comfortably above the rule
 * bound so probing stays short. */
#define AF_SLOTS 16384

static void af_ruleset_free(struct af_ruleset *rs)
{
	if (!rs)
		return;
	kvfree(rs->any_slots);
	kvfree(rs->per_slots);
	kfree(rs);
}

static struct af_ruleset *af_ruleset_alloc(void)
{
	struct af_ruleset *rs = kzalloc(sizeof(*rs), GFP_KERNEL);

	if (!rs)
		return NULL;
	rs->any_slots = kvcalloc(AF_SLOTS, sizeof(__u64), GFP_KERNEL);
	rs->per_slots = kvcalloc(AF_SLOTS, sizeof(__u64), GFP_KERNEL);
	if (!rs->any_slots || !rs->per_slots) {
		af_ruleset_free(rs);
		return NULL;
	}
	af_hashset_init(&rs->any, rs->any_slots, AF_SLOTS);
	af_hashset_init(&rs->per, rs->per_slots, AF_SLOTS);
	return rs;
}

/* Is this source MAC an enrolled subject? */
static bool af_is_subject(const struct af_ruleset *rs, const __u8 *mac)
{
	__u32 i;

	if (!mac)
		return false;
	for (i = 0; i < rs->n_macs; i++) {
		if (ether_addr_equal(rs->macs[i], mac))
			return true;
	}
	return false;
}

/* Source MAC, or NULL when the frame has no usable L2 header. */
static const __u8 *af_src_mac(const struct sk_buff *skb)
{
	if (!skb_mac_header_was_set(skb))
		return NULL;
	if (skb_mac_header(skb) + ETH_HLEN > skb->data)
		return NULL;
	return eth_hdr(skb)->h_source;
}

/*
 * Locate the TCP payload. Returns length, or 0.
 *
 * pskb_may_pull is what makes the subsequent reads safe -- without it a
 * non-linear skb would have us reading headers that are not in the linear
 * area, which is a class of bug that presents as random corruption.
 */
static __u32 af_tcp_payload(struct sk_buff *skb, const __u8 **out,
                            __u16 *dport)
{
	unsigned int nhoff, thoff, doff;
	struct tcphdr _th, *th;
	__u8 proto;

	if (skb->protocol == htons(ETH_P_IP)) {
		struct iphdr _iph, *iph;

		iph = skb_header_pointer(skb, skb_network_offset(skb),
		                         sizeof(_iph), &_iph);
		if (!iph || iph->version != 4)
			return 0;
		/* Non-first fragments carry no L4 header. */
		if (ntohs(iph->frag_off) & IP_OFFSET)
			return 0;
		proto = iph->protocol;
		nhoff = skb_network_offset(skb) + (iph->ihl * 4);
	} else if (skb->protocol == htons(ETH_P_IPV6)) {
		struct ipv6hdr _ip6, *ip6;

		ip6 = skb_header_pointer(skb, skb_network_offset(skb),
		                         sizeof(_ip6), &_ip6);
		if (!ip6)
			return 0;
		/* Extension headers are not walked; such a packet simply yields
		 * no name, which downgrades the record rather than corrupting
		 * it. */
		proto = ip6->nexthdr;
		nhoff = skb_network_offset(skb) + sizeof(_ip6);
	} else {
		return 0;
	}

	if (proto != IPPROTO_TCP)
		return 0;

	th = skb_header_pointer(skb, nhoff, sizeof(_th), &_th);
	if (!th)
		return 0;
	doff = th->doff * 4;
	if (doff < sizeof(struct tcphdr))
		return 0;
	*dport = ntohs(th->dest);

	thoff = nhoff + doff;
	if (thoff >= skb->len)
		return 0;

	if (!pskb_may_pull(skb, thoff))
		return 0;
	*out = skb->data + thoff;
	return skb->len - thoff;
}

static unsigned int af_hook(void *priv, struct sk_buff *skb,
                            const struct nf_hook_state *state)
{
	const struct af_ruleset *rs;
	const __u8 *payload = NULL;
	__u32 plen;
	__u16 dport = 0;
	char name[AF_MAX_NAME_LEN + 1];
	enum af_extract er;
	__u64 h;
	unsigned int verdict = NF_ACCEPT;
	const __u8 *mac;

	(void)priv;
	(void)state;

	if (!skb)
		return NF_ACCEPT;

	AF_STAT_INC(packets_seen);

	plen = af_tcp_payload(skb, &payload, &dport);
	if (!plen || !payload)
		return NF_ACCEPT;
	if (dport != AF_PORT_HTTPS && dport != AF_PORT_HTTP)
		return NF_ACCEPT;

	er = (dport == AF_PORT_HTTPS)
	         ? af_extract_sni(payload, plen, name, sizeof(name))
	         : af_extract_http_host(payload, plen, name, sizeof(name));
	if (er != AF_EXTRACT_OK) {
		/* Not an error in the common case -- most packets in a flow are
		 * not the one carrying the name. Counted because a rising rate
		 * is diagnostic long before it is an attack. */
		if (er == AF_EXTRACT_MALFORMED || er == AF_EXTRACT_TRUNCATED)
			AF_STAT_INC(parse_failed);
		return NF_ACCEPT;
	}

	AF_STAT_INC(names_extracted);
	h = af_hash_name(name, (__u32)strlen(name));

	rcu_read_lock();
	rs = rcu_dereference(af_active);
	if (rs) {
		if (af_hashset_contains(&rs->any, h)) {
			verdict = NF_DROP;
		} else if (rs->n_macs && af_hashset_contains(&rs->per, h)) {
			/* Subject-scoped: only drop for an enrolled MAC. A rule
			 * that silently applies household-wide is a failure mode
			 * ADR-017 names explicitly, so the scope is checked, never
			 * assumed. */
			mac = af_src_mac(skb);
			if (mac && af_is_subject(rs, mac))
				verdict = NF_DROP;
		}
	}
	rcu_read_unlock();

	if (verdict == NF_DROP) {
		AF_STAT_INC(matched);
		AF_STAT_INC(dropped);
		/* Silent drop. No nf_send_reset -- see the file header. */
	}
	return verdict;
}

static struct nf_hook_ops af_hook_ops[] = {
	{
		.hook = af_hook,
		.pf = NFPROTO_IPV4,
		.hooknum = NF_INET_FORWARD,
		.priority = NF_IP_PRI_FILTER - 1,
	},
	{
		.hook = af_hook,
		.pf = NFPROTO_IPV6,
		.hooknum = NF_INET_FORWARD,
		.priority = NF_IP6_PRI_FILTER - 1,
	},
};

/* ------------------------------------------------------------ netlink --- */

static void af_send_stats(__u32 portid)
{
	struct sk_buff *skb;
	struct nlmsghdr *nlh;
	struct af_msg_hdr *hdr;
	struct af_stats *out;
	size_t payload = sizeof(*hdr) + sizeof(*out);
	unsigned long flags;

	skb = nlmsg_new(payload, GFP_KERNEL);
	if (!skb)
		return;
	nlh = nlmsg_put(skb, 0, 0, AF_EVT_STATS, (int)payload, 0);
	if (!nlh) {
		nlmsg_free(skb);
		return;
	}
	hdr = nlmsg_data(nlh);
	hdr->type = AF_EVT_STATS;
	hdr->count = 1;
	hdr->len = sizeof(*out);
	out = (struct af_stats *)(hdr + 1);

	spin_lock_irqsave(&af_stats_lock, flags);
	*out = af_stats;
	spin_unlock_irqrestore(&af_stats_lock, flags);

	nlmsg_unicast(af_nl_sock, skb, portid);
}

static void af_handle_msg(struct sk_buff *skb)
{
	struct nlmsghdr *nlh = nlmsg_hdr(skb);
	struct af_msg_hdr *hdr;
	void *body;
	__u32 portid;

	if (!nlh || skb->len < NLMSG_HDRLEN)
		return;
	portid = nlh->nlmsg_pid;

	if (nlmsg_len(nlh) < (int)sizeof(*hdr))
		return;
	hdr = nlmsg_data(nlh);
	if (hdr->len > AF_MAX_MSG_LEN)
		return;
	if ((__u32)nlmsg_len(nlh) < sizeof(*hdr) + hdr->len)
		return;
	body = hdr + 1;

	switch (hdr->type) {
	case AF_MSG_HELLO: {
		struct af_hello *hi = body;

		if (hdr->len < sizeof(*hi))
			return;
		if (hi->version != AF_PROTO_VERSION) {
			/* Refuse rather than guess. A rule table parsed under
			 * the wrong version is worse than none, because it
			 * looks like it is working. */
			pr_warn("aether-af: protocol version %u refused, need %u\n",
			        hi->version, AF_PROTO_VERSION);
			return;
		}
		pr_info("aether-af: daemon connected, protocol v%u\n", hi->version);
		break;
	}
	case AF_MSG_RULES_BEGIN:
		mutex_lock(&af_stage_lock);
		af_ruleset_free(af_staging);
		af_staging = af_ruleset_alloc();
		mutex_unlock(&af_stage_lock);
		break;

	case AF_MSG_RULE_ADD: {
		struct af_rule *r = body;
		__u16 i, n = hdr->count;

		if (hdr->len < (__u32)n * sizeof(*r))
			return;
		mutex_lock(&af_stage_lock);
		if (!af_staging) {
			mutex_unlock(&af_stage_lock);
			return;
		}
		for (i = 0; i < n; i++) {
			struct af_hashset *set = r[i].per_subject
			                             ? &af_staging->per
			                             : &af_staging->any;
			if (!af_hashset_insert(set, r[i].name_hash))
				AF_STAT_INC(rules_refused);
		}
		mutex_unlock(&af_stage_lock);
		break;
	}
	case AF_MSG_SUBJECT_ADD: {
		struct af_subject *s = body;
		__u16 i, n = hdr->count;

		if (hdr->len < (__u32)n * sizeof(*s))
			return;
		mutex_lock(&af_stage_lock);
		if (!af_staging) {
			mutex_unlock(&af_stage_lock);
			return;
		}
		for (i = 0; i < n; i++) {
			if (af_staging->n_macs >= AF_MAX_SUBJECTS) {
				AF_STAT_INC(subjects_refused);
				continue;
			}
			ether_addr_copy(af_staging->macs[af_staging->n_macs],
			                s[i].mac);
			af_staging->n_macs++;
		}
		mutex_unlock(&af_stage_lock);
		break;
	}
	case AF_MSG_RULES_COMMIT: {
		struct af_ruleset *old, *new_rs;
		unsigned long flags;

		mutex_lock(&af_stage_lock);
		new_rs = af_staging;
		af_staging = NULL;
		mutex_unlock(&af_stage_lock);
		if (!new_rs)
			return;

		/* Atomic swap. The packet path never observes a partial set. */
		old = rcu_dereference_protected(af_active, true);
		rcu_assign_pointer(af_active, new_rs);
		if (old) {
			/*
			 * synchronize_net() rather than call_rcu().
			 *
			 * Both wait for readers to finish; only one of them is
			 * reachable without a GPL-only symbol. call_rcu and
			 * synchronize_rcu are EXPORT_SYMBOL_GPL, while
			 * synchronize_net is plain EXPORT_SYMBOL -- verified
			 * against Module.symvers for this exact kernel. Those
			 * two were the ONLY GPL-only symbols this module used;
			 * removing them leaves it with none.
			 *
			 * Blocking here is fine: commits arrive from the daemon
			 * over netlink in process context and are rare. Nothing
			 * on the packet path waits.
			 */
			synchronize_net();
			af_ruleset_free(old);
		}

		spin_lock_irqsave(&af_stats_lock, flags);
		af_stats.rules_loaded = new_rs->any.count + new_rs->per.count;
		af_stats.subjects_loaded = new_rs->n_macs;
		spin_unlock_irqrestore(&af_stats_lock, flags);

		pr_info("aether-af: committed %llu rules, %u subjects\n",
		        (unsigned long long)af_stats.rules_loaded, new_rs->n_macs);
		break;
	}
	case AF_MSG_SUBJECT_CLEAR:
		mutex_lock(&af_stage_lock);
		if (af_staging)
			af_staging->n_macs = 0;
		mutex_unlock(&af_stage_lock);
		break;

	case AF_MSG_STATS_REQ:
		af_send_stats(portid);
		break;

	default:
		break;
	}
}

static int __init af_init(void)
{
	struct netlink_kernel_cfg cfg = {
		.input = af_handle_msg,
	};
	int rc;

	af_nl_sock = netlink_kernel_create(&init_net, AF_NETLINK_UNIT, &cfg);
	if (!af_nl_sock) {
		pr_err("aether-af: cannot create netlink unit %d -- is another module using it?\n",
		       AF_NETLINK_UNIT);
		return -ENOMEM;
	}

	rc = nf_register_net_hooks(&init_net, af_hook_ops,
	                           ARRAY_SIZE(af_hook_ops));
	if (rc) {
		netlink_kernel_release(af_nl_sock);
		af_nl_sock = NULL;
		pr_err("aether-af: cannot register netfilter hooks: %d\n", rc);
		return rc;
	}

	pr_info("aether-af: loaded, netlink unit %d, protocol v%d. "
	        "No rules until the daemon sends them -- nothing is blocked yet.\n",
	        AF_NETLINK_UNIT, AF_PROTO_VERSION);
	return 0;
}

static void __exit af_exit(void)
{
	struct af_ruleset *rs;

	nf_unregister_net_hooks(&init_net, af_hook_ops, ARRAY_SIZE(af_hook_ops));
	if (af_nl_sock) {
		netlink_kernel_release(af_nl_sock);
		af_nl_sock = NULL;
	}

	rs = rcu_dereference_protected(af_active, true);
	RCU_INIT_POINTER(af_active, NULL);
	synchronize_net(); /* see the commit path: not synchronize_rcu */
	af_ruleset_free(rs);

	mutex_lock(&af_stage_lock);
	af_ruleset_free(af_staging);
	af_staging = NULL;
	mutex_unlock(&af_stage_lock);

	pr_info("aether-af: unloaded\n");
}

module_init(af_init);
module_exit(af_exit);
