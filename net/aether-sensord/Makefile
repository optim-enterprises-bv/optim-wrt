#
# Copyright (C) 2026 Optim Enterprises BV
#
# This is free software, licensed under the BSD 3-Clause License.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=aether-sensord
PKG_VERSION:=0.1.0
PKG_RELEASE:=21

# Source lives in its own repository, pinned rather than tracked: a moving
# branch would change what a tagged firmware contains without changing the
# feed. BUSL-1.1 -- source-available, production use granted for use with the
# Aether platform; see LICENSE in that repository.
PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/optim-enterprises-bv/aether-sensord.git
PKG_SOURCE_VERSION:=6d798fad67251d4c578dc9b7a103a36432e839d3
PKG_MIRROR_HASH:=skip
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)

# Source lives in this feed. See net/aether-fwlogs for the same rationale:
# a package-local src/ tree is cheaper to review than another repo plus a
# pinned commit, and it keeps the signature schema in the same change as the
# code that parses it.
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

PKG_MAINTAINER:=Optim Enterprises BV
PKG_LICENSE:=BUSL-1.1

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/aether-sensord
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Firewall
  TITLE:=Device-side security service (reputation enforcement + sensing)
  URL:=https://github.com/optim-enterprises-bv/optim-wrt
  # aether-appdb carries the signature database. Declared rather than assumed:
  # this daemon previously read a path that only another package happened to
  # install, so installing it alone loaded zero signatures and blocked nothing.
  # +libndpi is LGPL-3.0-or-later and links into this BSD-3-Clause binary
  # without contaminating it. It is the LIBRARY; the nDPId daemon (GPL-3,
  # cannot enforce) is deliberately not used.
  # kmod-nfnetlink-log: NFLOG cannot be bound without it and it is not in
  #   DEFAULT_PACKAGES. Its absence gives "cannot bind NFLOG group N (Invalid
  #   argument)" and both sensing and classification silently do nothing.
  # kmod-aether-af: the name-hash half of enforcement. The daemon still
  #   degrades gracefully when the module is not LOADED -- that is a different
  #   failure from it not being INSTALLED, and only the second one is a
  #   packaging question.
  DEPENDS:=+libc +nftables-json +aether-appdb +libndpi +kmod-nfnetlink-log \
           +kmod-aether-af
  # Supersedes the standalone sensor daemon, whose sources now live here.
  # PROVIDES lets an installed aether-fwlogs be replaced rather than sit
  # alongside this one feeding the same NFLOG group from a second process.
  PROVIDES:=aether-fwlogs
  CONFLICTS:=aether-fwlogs
endef

define Package/aether-sensord/description
  Applies the attacker-reputation set published by the aether controller
  (ADR-019) to nftables, and CONFIRMS it took effect.

  Feed messages arrive as files in a spool directory, delivered by ac-client,
  which already holds the device's mTLS identity. This daemon opens no network
  socket and holds no credential.

  Also carries firewall-drop sensing, formerly the separate aether-fwlogs
  package: one poll loop watches both the NFLOG socket and the feed spool, so
  there is one config surface, one identity and one health surface instead of
  two daemons that could disagree.

  Sensing is OFF by default and consented separately from enforcement. The
  enforcement half blocks hostile inbound and collects nothing about the
  subscriber; the sensing half reports attacker addresses, which are personal
  data. Merging the daemons did not merge their consent (ADR-019 section 9).

  The delta/serial/resync protocol means a missed update is recoverable: a gap
  leaves the set untouched and demands a snapshot, rather than applying across
  the hole and diverging from the controller invisibly from both ends.

  Every apply is followed by reading the kernel's own view of the set back.
  `nft -f` returning 0 means nft parsed a file; it does not mean the set exists
  or that a packet will die. Those are reported separately.

  Set declarations are installed as an fw4 include rather than created with
  `nft add set`, because fw4 rebuilds its ruleset on reload and would destroy
  an imperatively-created set. Elements carry a timeout, so reputation decays
  in the kernel as well as in the scorer.

  Also loads UCI application policy, resolves it against the signature
  database from aether-appdb, and compiles it to host-pattern hashes pushed to
  the aether-af kernel module over netlink. If any batch fails to send it does
  NOT commit, so the module keeps enforcing the last ruleset that committed
  whole.

  Links libndpi (LGPL-3.0-or-later) for protocol dissection: 474 protocols,
  QUIC, JA3/JA4, TLS certificate fields and per-flow risk flags. This is the
  LIBRARY; the nDPId daemon (GPL-3, libpcap, cannot enforce) is not used.

  Dissection decides WHERE a block can be applied. aether-af refuses anything
  that is not TCP and reads only TLS/HTTP, so a QUIC flow cannot be matched by
  name hash in the kernel no matter what nDPI extracted -- those are enforced
  by address in nftables instead. Replaying nDPI's own 349 test captures,
  66% of classified flows (3275 of 4936) fall on the address side.

  Classification is fed by its own NFLOG group from a non-terminating mirror
  rule on the forward chain, and flows are attributed to devices through
  /proc/net/arp and an RTM_GETNEIGH netlink dump. Attribution refuses rather
  than guesses: a stale or incomplete neighbour entry would blame whichever
  device previously held that address.

  CONSENTED SEPARATELY FROM SENSING, and for a stronger reason. Sensing takes
  128 bytes -- headers only, so packet contents never enter the daemon.
  Dissection needs the payload, because a TLS ClientHello or a QUIC Initial IS
  the payload. dpi_enabled is therefore its own option and defaults to off.

  FLOW OFFLOADING AND INSPECTION ARE MUTUALLY EXCLUSIVE. fw4 emits
  `flow offload @ft` as the first statement of the forward chain, ahead of every
  include point it offers, so an established flow bypasses netfilter and the TLS
  ClientHello is never seen. The daemon probes for a flowtable at start and each
  interval and logs OFFLOAD CONFLICT when one exists; offload_policy chooses
  between leaving it alone and turning offloading off, which works but costs
  routing throughput. No setting gives both.

  TWO KNOWN GAPS, stated because neither is visible from any counter:

  aether-af hooks NF_INET_FORWARD and parses TLS and HTTP only, so hardware NAT
  offload bypasses it entirely and QUIC conceals the SNI it reads. open-app-filter
  shipped uci-defaults forcing disable_hnat and disable_quic; nothing equivalent
  ships here yet.

  64% of the signature database matches only 'www.<domain>', an exact hostname
  that is usually a marketing site rather than the endpoint an application uses.
  See SIGNATURE-COVERAGE.md in the aether-sensord repository.
endef

define Package/aether-sensord/conffiles
/etc/config/aether-sensord
endef

define Package/aether-sensord/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/aether-sensord $(1)/usr/sbin/aether-sensord

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/etc/config/aether-sensord $(1)/etc/config/aether-sensord

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/etc/init.d/aether-sensord $(1)/etc/init.d/aether-sensord

	$(INSTALL_DIR) $(1)/usr/share/nftables.d/table-pre
endef

define Package/aether-sensord/postinst
#!/bin/sh
set -e

# Nothing to say when unpacking into an image rather than installing on a
# running device. Printing here writes into the ImageBuilder's tar stream and
# fails the build with "tar: Deleting non-header from archive".
[ -n "$${IPKG_INSTROOT}" ] && exit 0

echo ""
echo "aether-sensord installed."
echo ""
echo "Reputation enforcement is enabled by default: it blocks inbound traffic"
echo "from addresses the controller scores as hostile, and collects nothing"
echo "about the subscriber."
echo ""
echo "  /etc/init.d/aether-sensord enable"
echo "  /etc/init.d/aether-sensord start"
echo "  fw4 reload      # once, so the declared sets are created"
echo ""
echo "Confirm it is actually ENFORCING, not merely running -- a daemon that"
echo "starts cleanly while enforcing nothing is this project's known failure:"
echo "   logread -e aether-sensord"
echo "   nft list set inet fw4 aether_rep4"
echo ""
echo "The log line to look for is 'applied and verified'. 'NOT enforced'"
echo "means nft accepted the batch but the set did not receive it."
echo ""
endef

define Package/aether-sigtool
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Firewall
  TITLE:=Signature database inspector for aether-sensord
  URL:=https://github.com/optim-enterprises-bv/optim-wrt
  DEPENDS:=+libc
endef

define Package/aether-sigtool/description
  Loads the application signature database and reports what was ACCEPTED --
  not what was offered, and not that a config push succeeded.

  This serves the first on-device check for the aether-sensord design
  (ADR-020). The reference Open App Filter daemon fills a fixed 1024-entry
  table with no bounds check, so a 1,347-signature database corrupts memory
  past entry 1024 and segfaults around 1098 during traffic rather than at
  load. "The daemon started" therefore proves nothing about coverage, and a
  database that quietly loaded 1024 of 1347 entries looks identical to one
  that loaded all of them.

  Commands:
    aether-sigtool stats                 accepted apps/rules, refusals, format
    aether-sigtool lookup <tag>          resolve a controller-facing tag
    aether-sigtool classify <host> [port]  what would this flow classify as

  Applications are addressed by stable TAG, never by numeric id. The shipped
  database defines YouTube under both 11001 and 39037, and 27 other names
  repeat; those merge into one tag carrying every rule. Numeric ids are not
  usable across the cloud/device boundary -- upstream's database calls 11001
  Samba.

  Exits non-zero if any signature was refused for want of capacity.
endef

# Default Build/Prepare unpacks the tarball; sources arrive under src/.
define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_CPPFLAGS) \
		-std=gnu11 -Wall -Wextra \
		-o $(PKG_BUILD_DIR)/aether-sigtool \
		$(PKG_BUILD_DIR)/src/sigtool.c \
		$(PKG_BUILD_DIR)/src/sigdb.c \
		$(PKG_BUILD_DIR)/src/match.c \
		$(TARGET_LDFLAGS)
	$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_CPPFLAGS) \
		-std=gnu11 -Wall -Wextra \
		-o $(PKG_BUILD_DIR)/aether-sensord \
		$(PKG_BUILD_DIR)/src/sensord.c \
		$(PKG_BUILD_DIR)/src/sigdb.c \
		$(PKG_BUILD_DIR)/src/match.c \
		$(PKG_BUILD_DIR)/src/policy.c \
		$(PKG_BUILD_DIR)/src/nft.c \
		$(PKG_BUILD_DIR)/src/feed.c \
		$(PKG_BUILD_DIR)/src/apply.c \
		$(PKG_BUILD_DIR)/src/afpush.c \
		$(PKG_BUILD_DIR)/src/polcfg.c \
		$(PKG_BUILD_DIR)/src/dpi.c \
		$(PKG_BUILD_DIR)/src/appblock.c \
		$(PKG_BUILD_DIR)/src/neigh.c \
		$(PKG_BUILD_DIR)/src/offload.c \
		$(PKG_BUILD_DIR)/src/canary.c \
		$(PKG_BUILD_DIR)/src/nflog_raw.c \
		$(PKG_BUILD_DIR)/src/observe.c \
		$(PKG_BUILD_DIR)/src/flowlog.c \
		$(TARGET_LDFLAGS) -lndpi -lm -lpthread
endef

define Package/aether-sigtool/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/aether-sigtool $(1)/usr/bin/aether-sigtool
endef

# Both evals come last, AFTER Build/Prepare and Build/Compile above.
# BuildPackage expands the build rules at the point it is called, so a package
# evaluated before Build/Compile exists silently inherits the default autotools
# rule -- which runs ./configure and `make`, finds neither, and fails with an
# error that says nothing about the cause. That cost a build cycle here.
$(eval $(call BuildPackage,aether-sensord))
$(eval $(call BuildPackage,aether-sigtool))
