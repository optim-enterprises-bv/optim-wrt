/*
 * Copyright (C) 2026 Optim Enterprises BV
 *
 * This is free software, licensed under the BSD 3-Clause License.
 */

#include "nflog_raw.h"

#include <errno.h>
#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/nfnetlink_log.h>
#include <linux/netlink.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

/* NFULNL config commands are a nfgenmsg followed by attributes. */
struct nfr_nfgenmsg {
	uint8_t nfgen_family;
	uint8_t version;
	uint16_t res_id; /* group, big-endian */
};

static bool nfr_send_config(int fd, uint16_t group, uint8_t family,
                            uint8_t command, const void *extra,
                            size_t extra_len, uint16_t extra_type)
{
	uint8_t buf[256];
	struct nlmsghdr *nlh = (struct nlmsghdr *)buf;
	struct nfr_nfgenmsg *nfg;
	struct nlattr *attr;
	size_t off;
	struct sockaddr_nl dst;
	ssize_t n;

	memset(buf, 0, sizeof(buf));
	nlh->nlmsg_type = (uint16_t)((NFNL_SUBSYS_ULOG << 8) | NFULNL_MSG_CONFIG);
	nlh->nlmsg_flags = NLM_F_REQUEST;
	nlh->nlmsg_seq = 1;
	nlh->nlmsg_pid = 0;

	nfg = (struct nfr_nfgenmsg *)NLMSG_DATA(nlh);
	nfg->nfgen_family = family;
	nfg->version = NFNETLINK_V0;
	/* res_id carries the group and is big-endian on the wire. */
	nfg->res_id = (uint16_t)((group >> 8) | (group << 8));

	off = NLMSG_LENGTH(sizeof(*nfg));

	if (extra && extra_len) {
		size_t alen = NLA_HDRLEN + extra_len;

		if (off + NLA_ALIGN(alen) > sizeof(buf))
			return false;
		attr = (struct nlattr *)(buf + off);
		attr->nla_len = (uint16_t)alen;
		attr->nla_type = extra_type;
		memcpy((uint8_t *)attr + NLA_HDRLEN, extra, extra_len);
		off += NLA_ALIGN(alen);
	}
	(void)command;

	nlh->nlmsg_len = (uint32_t)off;

	memset(&dst, 0, sizeof(dst));
	dst.nl_family = AF_NETLINK;

	do {
		n = sendto(fd, buf, off, 0, (struct sockaddr *)&dst, sizeof(dst));
	} while (n < 0 && errno == EINTR);
	return n > 0;
}

bool nfr_open(struct nfr_conn *c, uint16_t group, uint16_t copy_range)
{
	struct sockaddr_nl sa;
	struct nfulnl_msg_config_cmd cmd;
	struct nfulnl_msg_config_mode mode;

	if (!c)
		return false;
	memset(c, 0, sizeof(*c));
	c->group = group;

	c->fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_NETFILTER);
	if (c->fd < 0)
		return false;

	memset(&sa, 0, sizeof(sa));
	sa.nl_family = AF_NETLINK;
	if (bind(c->fd, (struct sockaddr *)&sa, sizeof(sa)) != 0) {
		close(c->fd);
		c->fd = -1;
		return false;
	}

	/* PF_BIND for both families. A v6-only or v4-only deployment simply
	 * sees no traffic on the unused one. */
	memset(&cmd, 0, sizeof(cmd));
	cmd.command = NFULNL_CFG_CMD_PF_BIND;
	nfr_send_config(c->fd, 0, AF_INET, 0, &cmd, sizeof(cmd), NFULA_CFG_CMD);
	nfr_send_config(c->fd, 0, AF_INET6, 0, &cmd, sizeof(cmd), NFULA_CFG_CMD);

	/* Bind the group. */
	memset(&cmd, 0, sizeof(cmd));
	cmd.command = NFULNL_CFG_CMD_BIND;
	if (!nfr_send_config(c->fd, group, AF_UNSPEC, 0, &cmd, sizeof(cmd),
	                     NFULA_CFG_CMD)) {
		close(c->fd);
		c->fd = -1;
		return false;
	}

	/*
	 * Copy mode. Asking for only the headers is what keeps packet CONTENTS
	 * out of this process: we read addresses and ports, never payload.
	 */
	memset(&mode, 0, sizeof(mode));
	mode.copy_mode = NFULNL_COPY_PACKET;
	mode.copy_range = (uint32_t)((copy_range >> 24) | ((copy_range >> 8) & 0xff00) |
	                             ((copy_range << 8) & 0xff0000) |
	                             ((uint32_t)copy_range << 24));
	nfr_send_config(c->fd, group, AF_UNSPEC, 0, &mode, sizeof(mode),
	                NFULA_CFG_MODE);

	return true;
}

void nfr_close(struct nfr_conn *c)
{
	if (c && c->fd >= 0) {
		close(c->fd);
		c->fd = -1;
	}
}

long nfr_parse_buffer(struct nfr_conn *c, const uint8_t *buf, size_t len,
                      nfr_packet_fn cb, void *user)
{
	const struct nlmsghdr *nlh;
	size_t remaining = len;
	long dispatched = 0;

	if (!c || !buf || !cb)
		return 0;

	nlh = (const struct nlmsghdr *)buf;
	/* NLMSG_OK bounds every step against `remaining`; nothing below
	 * dereferences without it. */
	while (remaining >= sizeof(*nlh) && NLMSG_OK(nlh, remaining)) {
		uint16_t subsys = (uint16_t)(nlh->nlmsg_type >> 8);
		uint16_t mtype = (uint16_t)(nlh->nlmsg_type & 0xff);

		if (nlh->nlmsg_type == NLMSG_ERROR ||
		    nlh->nlmsg_type == NLMSG_DONE)
			break;

		if (subsys == NFNL_SUBSYS_ULOG && mtype == NFULNL_MSG_PACKET) {
			const uint8_t *p = (const uint8_t *)NLMSG_DATA(nlh);
			size_t plen = NLMSG_PAYLOAD(nlh, 0);
			const uint8_t *payload = NULL;
			uint32_t payload_len = 0;

			/* Skip the nfgenmsg that precedes the attributes. */
			if (plen < sizeof(struct nfr_nfgenmsg)) {
				c->malformed++;
				goto next;
			}
			p += sizeof(struct nfr_nfgenmsg);
			plen -= sizeof(struct nfr_nfgenmsg);

			/* Walk attributes. Every advance is bounded by plen. */
			while (plen >= NLA_HDRLEN) {
				const struct nlattr *a = (const struct nlattr *)p;
				uint16_t alen = a->nla_len;
				uint16_t atype = (uint16_t)(a->nla_type & NLA_TYPE_MASK);

				if (alen < NLA_HDRLEN || alen > plen) {
					c->malformed++;
					break;
				}
				if (atype == NFULA_PAYLOAD) {
					payload = p + NLA_HDRLEN;
					payload_len = (uint32_t)(alen - NLA_HDRLEN);
				}
				/* NLA_ALIGN yields a signed int on this libc; compare
				 * as size_t so a large aligned length cannot wrap
				 * the bound check and let the walk run on. */
				{
					size_t step = (size_t)NLA_ALIGN(alen);
					if (step > plen)
						break;
					p += step;
					plen -= step;
				}
			}

			if (payload && payload_len) {
				cb(payload, payload_len, user);
				dispatched++;
			}
		}
next:
		nlh = NLMSG_NEXT(nlh, remaining);
	}

	return dispatched;
}

long nfr_dispatch(struct nfr_conn *c, nfr_packet_fn cb, void *user)
{
	static uint8_t buf[NFR_RECV_BUF];
	ssize_t n;

	if (!c || c->fd < 0 || !cb)
		return -1;

	do {
		n = recv(c->fd, buf, sizeof(buf), 0);
	} while (n < 0 && errno == EINTR);

	if (n < 0) {
		if (errno == ENOBUFS) {
			/* The kernel queue overran and we lost records. Not
			 * fatal, but it must be visible: a sensor that hides
			 * its own data loss reports a quiet network during
			 * exactly the event that matters. */
			c->overruns++;
			return 0;
		}
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return 0;
		c->recv_errors++;
		return -1;
	}
	if (n == 0)
		return 0;

	return nfr_parse_buffer(c, buf, (size_t)n, cb, user);
}
