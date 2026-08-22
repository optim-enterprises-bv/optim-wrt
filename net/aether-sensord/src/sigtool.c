/*
 * Copyright (C) 2026 Optim Enterprises BV
 *
 * This is free software, licensed under the BSD 3-Clause License.
 *
 * aether-sigtool -- load the signature database and report what was ACCEPTED.
 *
 * This exists to serve ADR-020's first hardware gate item: confirm the engine
 * accepts all 1,347 shipped signatures, rather than confirming that a config
 * push succeeded. The reference implementation corrupts memory past entry 1024
 * and segfaults around 1098 *during traffic*, so "it started" proves nothing.
 *
 * Also answers, on-device and without the cloud:
 *   - what tag does the controller use for this app?
 *   - what would this hostname classify as?
 *
 * Usage:
 *   aether-sigtool [-d DB] stats
 *   aether-sigtool [-d DB] lookup <tag>
 *   aether-sigtool [-d DB] classify <host> [port]
 */

#include "match.h"
#include "sigdb.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_DB "/etc/appfilter/feature_en.cfg"

static int usage(const char *argv0)
{
	fprintf(stderr,
	        "usage: %s [-d DB] stats\n"
	        "       %s [-d DB] lookup <tag>\n"
	        "       %s [-d DB] classify <host> [port]\n",
	        argv0, argv0, argv0);
	return 2;
}

int main(int argc, char **argv)
{
	const char *path = DEFAULT_DB;
	int i = 1;

	if (i < argc && strcmp(argv[i], "-d") == 0) {
		if (i + 1 >= argc)
			return usage(argv[0]);
		path = argv[i + 1];
		i += 2;
	}
	if (i >= argc)
		return usage(argv[0]);

	const char *cmd = argv[i++];

	struct sig_db db;
	if (!sig_db_init(&db)) {
		fprintf(stderr, "cannot allocate signature tables\n");
		return 1;
	}

	long n = sig_db_load_path(&db, path);
	if (n < 0) {
		fprintf(stderr, "cannot read signature database: %s\n", path);
		sig_db_free(&db);
		return 1;
	}

	int rc = 0;

	if (strcmp(cmd, "stats") == 0) {
		printf("database    : %s\n", path);
		printf("format      : %s\n", db.format[0] ? db.format : "(undeclared)");
		printf("version     : %s\n", db.version[0] ? db.version : "(undeclared)");
		printf("apps        : %ld\n", n);
		printf("rules       : %zu\n", db.n_rules);
		printf("merged_by_tag: %u\n", db.merged_by_tag);
		printf("refused_malformed: %u\n", db.rejected_malformed);
		printf("refused_apps_full: %u  (capacity %d)\n",
		       db.rejected_apps_full, SIG_MAX_APPS);
		printf("refused_rules_full: %u  (capacity %d)\n",
		       db.rejected_rules_full, SIG_MAX_RULES);

		uint16_t max_class = 0;
		for (size_t k = 0; k < db.n_apps; k++)
			if (db.apps[k].class_id > max_class)
				max_class = db.apps[k].class_id;
		printf("highest_class: %u\n", max_class);

		/* The gate is on refusals, not on the headline count. A database
		 * that loaded 1024 of 1347 entries "successfully" is the exact
		 * failure this tool exists to make visible. */
		if (db.rejected_apps_full || db.rejected_rules_full) {
			printf("\nFAIL: signatures were refused for want of capacity.\n");
			rc = 1;
		} else if (db.rejected_malformed) {
			printf("\nWARN: %u malformed lines were skipped.\n",
			       db.rejected_malformed);
		} else {
			printf("\nOK: every line in the database was accounted for.\n");
		}
	} else if (strcmp(cmd, "lookup") == 0) {
		if (i >= argc) {
			rc = usage(argv[0]);
		} else {
			const struct sig_app *a = sig_db_by_tag(&db, argv[i]);
			if (!a) {
				printf("no such tag: %s\n", argv[i]);
				rc = 1;
			} else {
				printf("tag   : %s\n", a->tag);
				printf("name  : %s\n", a->name);
				printf("id    : %u\n", a->id);
				printf("class : %u\n", a->class_id);
				size_t idx = (size_t)(a - db.apps);
				for (size_t k = 0; k < db.n_rules; k++) {
					if (db.rules[k].app_index != idx)
						continue;
					printf("rule  : proto=%u sport=%u dport=%u host=%s\n",
					       db.rules[k].proto, db.rules[k].sport,
					       db.rules[k].dport,
					       db.rules[k].host[0] ? db.rules[k].host
					                           : "(port-only)");
				}
			}
		}
	} else if (strcmp(cmd, "classify") == 0) {
		if (i >= argc) {
			rc = usage(argv[0]);
		} else {
			const char *host = argv[i++];
			uint16_t port = (i < argc) ? (uint16_t)atoi(argv[i]) : 443;
			struct match_result m =
			    match_flow(&db, host, SIG_PROTO_TCP, port);
			if (!m.app) {
				printf("%s:%u -> unclassified\n", host, port);
				rc = 1;
			} else {
				printf("%s:%u -> tag=%s name=%s class=%u via=%s\n",
				       host, port, m.app->tag, m.app->name,
				       m.app->class_id, match_kind_str(m.kind));
			}
		}
	} else {
		rc = usage(argv[0]);
	}

	sig_db_free(&db);
	return rc;
}
