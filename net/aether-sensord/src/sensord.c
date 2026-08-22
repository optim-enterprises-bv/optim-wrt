/*
 * Copyright (C) 2026 Optim Enterprises BV
 *
 * This is free software, licensed under the BSD 3-Clause License.
 *
 * aether-sensord -- device-side security service (ADR-020).
 *
 * WHAT IS WIRED IN THIS BUILD: the attacker-reputation path, end to end.
 * Feed messages arrive as files in a spool directory, are parsed, folded into
 * the delta/serial state machine, rendered to nftables commands, applied, and
 * then VERIFIED against the kernel's own view of the set.
 *
 * WHAT IS NOT: capture, nDPI dissection and classification. The signature
 * database and matcher are loaded and reported here so the load can be
 * confirmed on-device, but nothing feeds them traffic yet, so no application
 * is classified and no app policy is enforced.
 *
 * `oaf.ko` continues to do app filtering, and IN THIS BUILD this daemon does
 * not talk to it. That is a statement about how far the work has got, not the
 * end state: ADR-020 decision 1 has aether-sensord replacing `oafd`, and
 * `oafd`'s entire job is driving `oaf.ko` over netlink id 29. So when the app
 * path is wired, this daemon WILL drive the module -- that is what replacing
 * the daemon means. What stays untouched is the kernel module itself, which
 * is consumed from upstream v6.1.8 rather than forked or renamed (ADR-020
 * decision 2): it is free, current, and does the in-kernel first-packet drop
 * that a userspace classifier cannot.
 *
 * That split is stated plainly rather than implied, because a daemon that
 * starts cleanly while enforcing nothing is the exact failure this project
 * keeps finding.
 *
 * The daemon does NOT talk to the network. Feed messages are delivered into
 * the spool by ac-client, which already holds the device's mTLS identity
 * (ADR-018); a second daemon with a second certificate store would be a second
 * thing to get wrong.
 */

#include "apply.h"
#include "feed.h"
#include "match.h"
#include "nft.h"
#include "policy.h"
#include "sigdb.h"

#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#define DEFAULT_DB "/etc/appfilter/feature_en.cfg"
#define DEFAULT_SPOOL "/var/spool/aether-sensord/feed"
#define DEFAULT_NFT_INCLUDE \
	"/usr/share/nftables.d/table-pre/inet/fw4/10-aether-sensord.nft"
#define DEFAULT_INTERVAL 30
#define DEFAULT_TIMEOUT_SEC 604800 /* 7 days, matching the scorer's half-life */

#define CMD_BUF 65536

static volatile sig_atomic_t running = 1;

static void on_signal(int sig)
{
	(void)sig;
	running = 0;
}

struct config {
	const char *db_path;
	const char *spool_dir;
	const char *nft_include;
	unsigned interval;
	uint32_t set_timeout;
	int foreground;
	int once;
	/* Overridable so the whole apply+verify path can be exercised against a
	 * stub without root, a device, or a real ruleset. */
	const char *nft_path;
};

/*
 * Write the set declarations to an fw4 include.
 *
 * Not `nft add set`: fw4 rebuilds its ruleset on every reload and an
 * imperatively-created set is destroyed the next time anything touches the
 * firewall. Writing to the include directory is what makes the set survive,
 * and it is an explicit ADR-020 gate item.
 */
static bool install_set_decl(const struct config *cfg,
                             const struct nft_target *t)
{
	char decl[4096];
	if (nft_render_set_decl(t, cfg->set_timeout, decl, sizeof(decl)) == 0) {
		syslog(LOG_ERR, "set declaration did not fit its buffer");
		return false;
	}

	char tmp[512];
	snprintf(tmp, sizeof(tmp), "%s.partial", cfg->nft_include);

	FILE *fp = fopen(tmp, "w");
	if (!fp) {
		syslog(LOG_ERR, "cannot write %s: %s -- is nftables.d present?", tmp,
		       strerror(errno));
		return false;
	}
	fputs(decl, fp);
	int err = ferror(fp);
	if (fclose(fp) != 0 || err) {
		syslog(LOG_ERR, "write failed for %s", tmp);
		unlink(tmp);
		return false;
	}
	/* Rename only after a complete write, so fw4 never reads a half file. */
	if (rename(tmp, cfg->nft_include) != 0) {
		syslog(LOG_ERR, "cannot install %s: %s", cfg->nft_include,
		       strerror(errno));
		unlink(tmp);
		return false;
	}
	return true;
}

/* Apply one parsed message. Returns true when the set was verified afterwards. */
static bool apply_message(struct apply_ctx *ap, const struct nft_target *t,
                          const struct feed_msg *msg, bool is_list)
{
	char cmds[CMD_BUF];
	size_t used = 0;
	size_t rendered = 0;
	char err[512];

	if (is_list) {
		size_t n = nft_render_flush(t, cmds, sizeof(cmds));
		if (n == 0)
			return false;
		used = n;
	}

	if (msg->n_remove > 0) {
		size_t n = nft_render_del(t, msg->remove, msg->n_remove, cmds + used,
		                          sizeof(cmds) - used, &rendered);
		if (n == 0 && msg->n_remove > 0) {
			syslog(LOG_WARNING, "removal batch did not fit; skipping batch");
			return false;
		}
		used += n;
	}

	size_t added = 0;
	if (msg->n_add > 0) {
		size_t n = nft_render_add(t, msg->add, msg->n_add, cmds + used,
		                          sizeof(cmds) - used, &added);
		if (n == 0) {
			syslog(LOG_WARNING, "addition batch did not fit; skipping batch");
			return false;
		}
		used += n;
	}

	if (used == 0)
		return true; /* nothing to do */

	/*
	 * Verify against a FLOOR, not an exact count: auto-merge legitimately
	 * collapses overlapping prefixes. For a delta we cannot predict the
	 * resulting total at all, so we only assert the set is readable and
	 * non-empty when we just added to it.
	 */
	long expect = (added > 0) ? 1 : 0;
	if (!apply_and_verify(ap, t, cmds, false, expect, err, sizeof(err))) {
		syslog(LOG_ERR, "reputation set NOT enforced: %s", err);
		return false;
	}
	return true;
}

/* Read one spool file, apply it, and remove it on success. */
static void process_file(const struct config *cfg, struct feed_client *fc,
                         struct apply_ctx *ap, const struct nft_target *t,
                         const char *path)
{
	FILE *fp = fopen(path, "r");
	if (!fp) {
		syslog(LOG_WARNING, "cannot open %s: %s", path, strerror(errno));
		return;
	}
	static char buf[CMD_BUF];
	size_t n = fread(buf, 1, sizeof(buf) - 1, fp);
	fclose(fp);
	buf[n] = '\0';
	if (n == 0) {
		unlink(path);
		return;
	}

	struct feed_msg msg;
	if (!feed_parse(buf, n, &msg)) {
		syslog(LOG_WARNING, "unusable feed message %s -- discarding", path);
		unlink(path);
		return;
	}
	if (msg.rejected || msg.overflowed)
		syslog(LOG_WARNING,
		       "feed serial %llu: %u elements refused, %u over capacity",
		       (unsigned long long)msg.serial, msg.rejected, msg.overflowed);

	enum feed_outcome o = feed_client_accept(fc, &msg);
	switch (o) {
	case FEED_STALE:
		unlink(path);
		return;
	case FEED_RESYNC_REQUIRED:
		/*
		 * Leave the set alone. Applying across a gap diverges the device
		 * from the controller invisibly from both ends.
		 */
		syslog(LOG_WARNING,
		       "feed gap at serial %llu (have %llu, missed %u) -- set left "
		       "untouched, awaiting a snapshot",
		       (unsigned long long)msg.serial,
		       (unsigned long long)fc->serial, fc->missed);
		unlink(path);
		return;
	case FEED_APPLIED:
		break;
	}

	if (apply_message(ap, t, &msg, msg.type == FEED_MSG_LIST)) {
		syslog(LOG_INFO, "feed serial %llu applied and verified (+%zu -%zu)",
		       (unsigned long long)msg.serial, msg.n_add, msg.n_remove);
		unlink(path);
	} else {
		/*
		 * Leave the file in place: the serial has advanced in memory but
		 * the kernel did not take it, so a restart should retry rather
		 * than skip. Losing the message here would leave a permanent
		 * silent divergence.
		 */
		syslog(LOG_ERR, "feed serial %llu retained for retry",
		       (unsigned long long)msg.serial);
	}
	(void)cfg;
}

static void scan_spool(const struct config *cfg, struct feed_client *fc,
                       struct apply_ctx *ap, const struct nft_target *t)
{
	DIR *d = opendir(cfg->spool_dir);
	if (!d)
		return;
	struct dirent *ent;
	while ((ent = readdir(d)) != NULL) {
		if (ent->d_name[0] == '.')
			continue;
		size_t len = strlen(ent->d_name);
		if (len < 6 || strcmp(ent->d_name + len - 5, ".json") != 0)
			continue;
		char path[1024];
		snprintf(path, sizeof(path), "%s/%s", cfg->spool_dir, ent->d_name);
		process_file(cfg, fc, ap, t, path);
	}
	closedir(d);
}

static void usage(const char *a0)
{
	fprintf(stderr,
	        "usage: %s [-d db] [-s spool] [-n nft-include] [-i interval]\n"
	        "          [-T set-timeout-sec] [-N nft-path] [-f] [-1]\n",
	        a0);
}

int main(int argc, char **argv)
{
	struct config cfg = {
		.db_path = DEFAULT_DB,
		.spool_dir = DEFAULT_SPOOL,
		.nft_include = DEFAULT_NFT_INCLUDE,
		.interval = DEFAULT_INTERVAL,
		.set_timeout = DEFAULT_TIMEOUT_SEC,
		.foreground = 0,
		.once = 0,
		.nft_path = NULL,
	};

	int opt;
	while ((opt = getopt(argc, argv, "d:s:n:i:T:N:f1h")) != -1) {
		switch (opt) {
		case 'd': cfg.db_path = optarg; break;
		case 's': cfg.spool_dir = optarg; break;
		case 'n': cfg.nft_include = optarg; break;
		case 'i': cfg.interval = (unsigned)strtoul(optarg, NULL, 10); break;
		case 'T': cfg.set_timeout = (uint32_t)strtoul(optarg, NULL, 10); break;
		case 'N': cfg.nft_path = optarg; break;
		case 'f': cfg.foreground = 1; break;
		case '1': cfg.once = 1; break;
		default: usage(argv[0]); return 2;
		}
	}
	if (cfg.interval == 0)
		cfg.interval = DEFAULT_INTERVAL;

	openlog("aether-sensord", cfg.foreground ? LOG_PERROR : 0, LOG_DAEMON);

	/*
	 * Load the signature database and report what was ACCEPTED. Nothing
	 * classifies traffic in this build, but the load is reported here so the
	 * ADR-020 gate item is checkable on a running device rather than only
	 * through aether-sigtool.
	 */
	struct sig_db sigs;
	if (!sig_db_init(&sigs)) {
		syslog(LOG_ERR, "cannot allocate signature tables");
		return 1;
	}
	long n_apps = sig_db_load_path(&sigs, cfg.db_path);
	if (n_apps < 0) {
		syslog(LOG_WARNING,
		       "signature database %s unreadable -- reputation enforcement "
		       "continues, application classification would not",
		       cfg.db_path);
	} else {
		syslog(LOG_INFO,
		       "signatures: %ld apps, %zu rules accepted, %u merged by tag, "
		       "%u malformed, %u refused for capacity (format %s)",
		       n_apps, sigs.n_rules, sigs.merged_by_tag,
		       sigs.rejected_malformed,
		       sigs.rejected_apps_full + sigs.rejected_rules_full,
		       sigs.format[0] ? sigs.format : "undeclared");
		if (sigs.rejected_apps_full || sigs.rejected_rules_full)
			syslog(LOG_ERR,
			       "SIGNATURES REFUSED FOR CAPACITY -- coverage is reduced "
			       "and this is not a warning to ignore");
	}

	struct nft_target target = { "inet", "fw4", "aether_rep4", "aether_rep6" };

	if (!install_set_decl(&cfg, &target)) {
		syslog(LOG_ERR, "cannot install nftables set declaration; refusing to "
		                "start rather than run with nowhere to enforce");
		sig_db_free(&sigs);
		return 1;
	}
	syslog(LOG_INFO, "set declaration installed at %s (run `fw4 reload` if the "
	                 "sets are not present yet)", cfg.nft_include);

	mkdir(cfg.spool_dir, 0750);

	/*
	 * Serial state is deliberately NOT persisted across restarts.
	 *
	 * On start we have no baseline, so the first delta triggers a resync and
	 * the controller sends a snapshot. That costs one extra message and is
	 * the correct behaviour, not merely the safe one: after a restart we do
	 * not know whether the nftables set survived. An `fw4 reload`, a
	 * firmware upgrade or a crash can empty it while a persisted serial
	 * would claim we are up to date -- and we would then apply deltas onto
	 * an empty set and believe the fleet was protected.
	 *
	 * Failing toward a snapshot trades a message for the guarantee that
	 * what the controller thinks is enforced actually is.
	 */
	struct feed_client fc;
	feed_client_init(&fc);

	struct apply_ctx ap;
	apply_ctx_init(&ap, apply_exec_posix, NULL);
	if (cfg.nft_path)
		ap.nft_path = cfg.nft_path;

	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = on_signal;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	syslog(LOG_INFO,
	       "started: spool=%s interval=%us set_timeout=%us. Reputation "
	       "enforcement is live; application classification is NOT wired in "
	       "this build.",
	       cfg.spool_dir, cfg.interval, cfg.set_timeout);

	do {
		scan_spool(&cfg, &fc, &ap, &target);

		if (feed_client_needs_resync(&fc))
			syslog(LOG_WARNING,
			       "awaiting a full snapshot: %u updates missed since serial "
			       "%llu",
			       fc.missed, (unsigned long long)fc.serial);

		if (cfg.once)
			break;
		for (unsigned s = 0; s < cfg.interval && running; s++)
			sleep(1);
	} while (running);

	syslog(LOG_INFO,
	       "stopping: %llu deltas, %llu snapshots applied, %llu resyncs, "
	       "%llu batches applied, %llu failed, %llu verify mismatches",
	       (unsigned long long)fc.applied_deltas,
	       (unsigned long long)fc.applied_lists,
	       (unsigned long long)fc.resyncs,
	       (unsigned long long)ap.applied_batches,
	       (unsigned long long)ap.failed_batches,
	       (unsigned long long)ap.verify_mismatches);

	sig_db_free(&sigs);
	closelog();
	return 0;
}
