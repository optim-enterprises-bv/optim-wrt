#
# Copyright (C) 2026 Optim Enterprises BV
#
# This is free software, licensed under the BSD 3-Clause License.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=aether-sensord
PKG_VERSION:=0.1.0
PKG_RELEASE:=1

# Source lives in this feed. See net/aether-fwlogs for the same rationale:
# a package-local src/ tree is cheaper to review than another repo plus a
# pinned commit, and it keeps the signature schema in the same change as the
# code that parses it.
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

PKG_MAINTAINER:=Optim Enterprises BV
PKG_LICENSE:=BSD-3-Clause

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/aether-sensord
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Firewall
  TITLE:=Device-side security service (reputation enforcement)
  URL:=https://github.com/optim-enterprises-bv/optim-wrt
  DEPENDS:=+libc +nftables-json
endef

define Package/aether-sensord/description
  Applies the attacker-reputation set published by the aether controller
  (ADR-019) to nftables, and CONFIRMS it took effect.

  Feed messages arrive as files in a spool directory, delivered by ac-client,
  which already holds the device's mTLS identity. This daemon opens no network
  socket and holds no credential.

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

  NOT IN THIS BUILD: capture, nDPI dissection, application classification and
  app policy enforcement. The signature database is loaded and reported so the
  load is confirmable on-device, but nothing feeds it traffic. Application
  filtering continues to be done by oaf.ko, which this daemon does not touch.
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

	$(INSTALL_DIR) $(1)/usr/share/nftables.d/table-pre/inet/fw4
endef

define Package/aether-sensord/postinst
#!/bin/sh
set -e

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

$(eval $(call BuildPackage,aether-sensord))

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

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_CPPFLAGS) \
		-std=gnu11 -Wall -Wextra \
		-o $(PKG_BUILD_DIR)/aether-sigtool \
		$(PKG_BUILD_DIR)/sigtool.c \
		$(PKG_BUILD_DIR)/sigdb.c \
		$(PKG_BUILD_DIR)/match.c \
		$(TARGET_LDFLAGS)
	$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_CPPFLAGS) \
		-std=gnu11 -Wall -Wextra \
		-o $(PKG_BUILD_DIR)/aether-sensord \
		$(PKG_BUILD_DIR)/sensord.c \
		$(PKG_BUILD_DIR)/sigdb.c \
		$(PKG_BUILD_DIR)/match.c \
		$(PKG_BUILD_DIR)/policy.c \
		$(PKG_BUILD_DIR)/nft.c \
		$(PKG_BUILD_DIR)/feed.c \
		$(PKG_BUILD_DIR)/apply.c \
		$(TARGET_LDFLAGS)
endef

define Package/aether-sigtool/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/aether-sigtool $(1)/usr/bin/aether-sigtool
endef

$(eval $(call BuildPackage,aether-sigtool))
