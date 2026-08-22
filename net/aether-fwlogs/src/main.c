/*
 * Copyright (C) 2026 Optim Enterprises BV
 *
 * This is free software, licensed under the BSD 3-Clause License.
 *
 * aether-fwlogs -- firewall-drop sensor.
 *
 * Reads packets the firewall already dropped or rejected via NFLOG, aggregates
 * them per source address, and writes periodic NDJSON batches to a spool
 * directory. It does not talk to the network: shipping the spool is ac-client's
 * job, because ac-client already holds the device's mTLS identity and a second
 * daemon with a second certificate store would be a second thing to get wrong.
 *
 * It observes; it does not bait. Every record here describes a packet the
 * device was already going to see and already going to drop.
 */

#include "observe.h"

/* recv() is declared in <sys/socket.h>, which nothing else here pulls in
 * reliably across glibc and musl. */
#include <sys/socket.h>
#include <sys/time.h>

#include <arpa/inet.h>
#include <errno.h>
#include <getopt.h>
#include "nflog_raw.h"
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#define DEFAULT_GROUP 5
#define DEFAULT_CAPACITY 4096
#define DEFAULT_INTERVAL 300
#define DEFAULT_SCAN_PORTS 8
#define DEFAULT_SPOOL_DIR "/var/spool/aether-fwlogs"
/* Bounded so a device that never gets drained does not fill its tmpfs. Oldest
 * batches are removed first: recent hostility is what the backend scores. */
#define DEFAULT_MAX_BATCHES 32

struct config {
	unsigned group;
	size_t capacity;
	unsigned interval;
	uint32_t scan_ports;
	const char *spool_dir;
	unsigned max_batches;
	int foreground;
};

static volatile sig_atomic_t running = 1;
static volatile sig_atomic_t flush_now;
static struct obs_table table;

static void on_signal(int sig)
{
	if (sig == SIGUSR1)
		flush_now = 1;
	else
		running = 0;
}

static void on_packet(const uint8_t *payload, uint32_t plen, void *user)
{
	struct obs_addr src;
	uint16_t dport = 0;

	(void)user;
	if (!payload || plen == 0)
		return;
	if (obs_decode(payload, (int)plen, &src, &dport) != 0)
		return;

	obs_record(&table, &src, dport, (int64_t)time(NULL));
}

/* Keep at most `max_batches` spool files, deleting oldest first. */
static void prune_spool(const struct config *cfg)
{
	/* Implemented with a shell glob rather than scandir to keep the binary
	 * small; the spool is tiny and this runs once per interval. */
	char cmd[512];
	snprintf(cmd, sizeof(cmd),
	         "ls -1t %s/batch-*.ndjson 2>/dev/null | tail -n +%u | xargs -r rm -f",
	         cfg->spool_dir, cfg->max_batches + 1);
	if (system(cmd) != 0)
		syslog(LOG_DEBUG, "spool prune returned non-zero");
}

static void flush_batch(const struct config *cfg)
{
	if (table.used == 0 && table.dropped_full == 0 && table.dropped_private == 0)
		return;

	char path[512];
	char tmp[540];
	time_t now = time(NULL);
	snprintf(path, sizeof(path), "%s/batch-%lld.ndjson", cfg->spool_dir,
	         (long long)now);
	snprintf(tmp, sizeof(tmp), "%s.partial", path);

	FILE *f = fopen(tmp, "w");
	if (!f) {
		syslog(LOG_ERR, "cannot open spool file %s: %s", tmp, strerror(errno));
		return;
	}

	long n = obs_write_ndjson(&table, f);

	/* Counters ride along in the batch so the backend can tell "quiet
	 * network" from "sensor overwhelmed" or "firewall rule mis-scoped".
	 * A sensor that hides its own degradation is worse than no sensor. */
	fprintf(f,
	        "{\"meta\":true,\"sources\":%ld,\"dropped_full\":%llu,"
	        "\"dropped_private\":%llu,\"interval\":%u,\"emitted_at\":%lld}\n",
	        n < 0 ? 0 : n, (unsigned long long)table.dropped_full,
	        (unsigned long long)table.dropped_private, cfg->interval,
	        (long long)now);

	int err = ferror(f);
	if (fclose(f) != 0 || err || n < 0) {
		syslog(LOG_ERR, "spool write failed, discarding batch");
		unlink(tmp);
		obs_table_reset(&table);
		return;
	}

	/* Rename only after a complete write, so a consumer never sees a
	 * half-written batch. */
	if (rename(tmp, path) != 0) {
		syslog(LOG_ERR, "cannot rename %s: %s", tmp, strerror(errno));
		unlink(tmp);
	} else {
		syslog(LOG_INFO, "wrote %ld source records to %s", n, path);
		if (table.dropped_private > 0)
			syslog(LOG_WARNING,
			       "%llu private-source records refused -- the NFLOG rule "
			       "is logging LAN traffic and should be WAN-scoped",
			       (unsigned long long)table.dropped_private);
		if (table.dropped_full > 0)
			syslog(LOG_WARNING,
			       "%llu records dropped, table full (capacity %zu)",
			       (unsigned long long)table.dropped_full, table.capacity);
	}

	prune_spool(cfg);
	obs_table_reset(&table);
}

static void usage(const char *argv0)
{
	fprintf(stderr,
	        "usage: %s [-g group] [-c capacity] [-i interval] [-p scan_ports]\n"
	        "          [-d spool_dir] [-b max_batches] [-f]\n",
	        argv0);
}

int main(int argc, char **argv)
{
	struct config cfg = {
		.group = DEFAULT_GROUP,
		.capacity = DEFAULT_CAPACITY,
		.interval = DEFAULT_INTERVAL,
		.scan_ports = DEFAULT_SCAN_PORTS,
		.spool_dir = DEFAULT_SPOOL_DIR,
		.max_batches = DEFAULT_MAX_BATCHES,
		.foreground = 0,
	};

	int opt;
	while ((opt = getopt(argc, argv, "g:c:i:p:d:b:fh")) != -1) {
		switch (opt) {
		case 'g': cfg.group = (unsigned)strtoul(optarg, NULL, 10); break;
		case 'c': cfg.capacity = (size_t)strtoul(optarg, NULL, 10); break;
		case 'i': cfg.interval = (unsigned)strtoul(optarg, NULL, 10); break;
		case 'p': cfg.scan_ports = (uint32_t)strtoul(optarg, NULL, 10); break;
		case 'd': cfg.spool_dir = optarg; break;
		case 'b': cfg.max_batches = (unsigned)strtoul(optarg, NULL, 10); break;
		case 'f': cfg.foreground = 1; break;
		default: usage(argv[0]); return 2;
		}
	}

	if (cfg.capacity == 0 || cfg.interval == 0) {
		usage(argv[0]);
		return 2;
	}

	openlog("aether-fwlogs", cfg.foreground ? LOG_PERROR : 0, LOG_DAEMON);

	if (mkdir(cfg.spool_dir, 0750) != 0 && errno != EEXIST) {
		syslog(LOG_ERR, "cannot create spool dir %s: %s", cfg.spool_dir,
		       strerror(errno));
		return 1;
	}

	if (cfg.scan_ports > OBS_MAX_PORTS) {
		syslog(LOG_ERR,
		       "scan_ports=%u exceeds the port store (%d); a threshold above "
		       "it can never be reached and scan detection would be silently "
		       "disabled. Refusing to start.",
		       cfg.scan_ports, OBS_MAX_PORTS);
		return 1;
	}
	if (!obs_table_init(&table, cfg.capacity, cfg.scan_ports)) {
		syslog(LOG_ERR, "cannot initialise observation table");
		return 1;
	}

	struct nfr_conn nfl;
	/* 96 bytes is enough for the L3/L4 headers and no more. Asking for only
	 * that is what keeps packet CONTENTS out of this process entirely -- we
	 * read addresses and ports, never payload. */
	if (!nfr_open(&nfl, (uint16_t)cfg.group, 96)) {
		syslog(LOG_ERR,
		       "cannot bind NFLOG group %u: %s -- is another daemon already "
		       "bound to it, or is kmod-nfnetlink-log missing?",
		       cfg.group, strerror(errno));
		return 1;
	}

	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = on_signal;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGUSR1, &sa, NULL);

	time_t last_flush = time(NULL);

	syslog(LOG_INFO,
	       "started: group=%u capacity=%zu interval=%us scan_ports=%u spool=%s",
	       cfg.group, cfg.capacity, cfg.interval, cfg.scan_ports, cfg.spool_dir);

	while (running) {
		long got = nfr_dispatch(&nfl, on_packet, NULL);
		if (got < 0) {
			syslog(LOG_ERR, "NFLOG receive failed: %s", strerror(errno));
			break;
		}

		time_t now = time(NULL);
		if (flush_now || (now - last_flush) >= (time_t)cfg.interval) {
			flush_now = 0;
			flush_batch(&cfg);
			last_flush = now;
			if (nfl.overruns) {
				syslog(LOG_WARNING,
				       "%llu NFLOG queue overruns -- records were lost",
				       (unsigned long long)nfl.overruns);
				nfl.overruns = 0;
			}
			if (nfl.malformed) {
				syslog(LOG_WARNING,
				       "%llu malformed NFLOG messages",
				       (unsigned long long)nfl.malformed);
				nfl.malformed = 0;
			}
		}
	}

	syslog(LOG_INFO, "shutting down, flushing final batch");
	flush_batch(&cfg);

	nfr_close(&nfl);
	obs_table_free(&table);
	closelog();
	return 0;
}
