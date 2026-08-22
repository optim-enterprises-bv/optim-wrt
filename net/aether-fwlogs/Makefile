#
# Copyright (C) 2026 Optim Enterprises BV
#
# This is free software, licensed under the BSD 3-Clause License.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=aether-fwlogs
PKG_VERSION:=0.1.0
PKG_RELEASE:=1

# Source lives in this feed rather than in a separate repository. At ~700
# lines with one external dependency, a package-local src/ tree is cheaper to
# review and to build than another repo + pinned commit + deploy key, and it
# keeps the sensor's report schema in the same change as the config that
# produces it. Revisit if this grows a second consumer.
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

PKG_MAINTAINER:=Optim Enterprises BV
PKG_LICENSE:=BSD-3-Clause

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/aether-fwlogs
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Firewall
  TITLE:=Firewall-drop sensor for aether attacker reputation
  URL:=https://github.com/optim-enterprises-bv/optim-wrt
  DEPENDS:=+libnetfilter-log +libnfnetlink +kmod-nfnetlink-log
endef

define Package/aether-fwlogs/description
  Reads packets the firewall has already dropped or rejected via NFLOG,
  aggregates them per source address, and writes periodic NDJSON batches to
  a spool directory for upload.

  It reports (source address, destination port, counts, first/last seen) and
  classifies each source as a blocked connection or a port scan. Only 96
  bytes of each packet are copied from the kernel -- enough for the L3/L4
  headers, so packet contents are never read.

  This daemon does NOT talk to the network. Shipping the spool is
  ac-client's job: ac-client already holds the device's mTLS identity, and a
  second daemon with a second certificate store is a second thing to get
  wrong.

  It runs no services and offers no bait. Every record describes traffic the
  device was already going to see and already going to drop. There is
  deliberately no honeypot component.

  PRIVACY: collection is DISABLED by default. The records are attacker IP
  addresses, which are personal data under GDPR; enabling this needs a
  lawful basis, not just a config change. The daemon additionally refuses
  private and reserved source addresses and counts the refusals, so a
  mis-scoped firewall rule that logs LAN traffic is visible rather than
  silently uploaded.
endef

define Package/aether-fwlogs/conffiles
/etc/config/aether-fwlogs
endef

# The package-local sources are copied into the build dir; there is no
# fetched tarball, so Build/Prepare is defined rather than inherited.
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_CPPFLAGS) \
		-std=gnu11 -Wall -Wextra \
		-o $(PKG_BUILD_DIR)/aether-fwlogs \
		$(PKG_BUILD_DIR)/main.c $(PKG_BUILD_DIR)/observe.c \
		$(TARGET_LDFLAGS) -lnetfilter_log -lnfnetlink
endef

define Package/aether-fwlogs/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/aether-fwlogs $(1)/usr/sbin/aether-fwlogs

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/etc/config/aether-fwlogs $(1)/etc/config/aether-fwlogs

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/etc/init.d/aether-fwlogs $(1)/etc/init.d/aether-fwlogs

	# NOTE: /var on OpenWrt is a symlink to /tmp (tmpfs). Do NOT install
	# the spool directory here -- the build system copies the package tree
	# with `cp -fpR` and hits the symlink with EISDIR. The init script
	# creates the spool directory at runtime.
endef

define Package/aether-fwlogs/postinst
#!/bin/sh
set -e

echo ""
echo "aether-fwlogs installed -- and DISABLED."
echo ""
echo "This sensor records the source addresses of packets your firewall"
echo "drops. Those are personal data under GDPR. Enabling collection is a"
echo "decision that needs a lawful basis, which is why it is not a default."
echo ""
echo "To enable:"
echo "   uci set aether-fwlogs.main.enabled='1'"
echo "   uci commit aether-fwlogs"
echo "   /etc/init.d/aether-fwlogs enable"
echo "   /etc/init.d/aether-fwlogs start"
echo ""
echo "Batches are written to /var/spool/aether-fwlogs as NDJSON. Nothing"
echo "uploads them yet -- the spool consumer in ac-client is separate work."
echo "Until it exists, this fills a bounded spool and is drained by hand."
echo ""
echo "Verify it is actually seeing traffic (the failure mode is a rule that"
echo "installs cleanly and matches nothing):"
echo "   logread -e aether-fwlogs"
echo "   nft list chain inet fw4 input | grep aether"
echo "   kill -USR1 \$(pgrep aether-fwlogs)   # force an immediate batch"
echo "   ls -l /var/spool/aether-fwlogs/"
echo ""
echo "If batches report a non-zero dropped_private count, the NFLOG rule is"
echo "logging LAN traffic and the configured zone is wrong."
echo ""
endef

$(eval $(call BuildPackage,aether-fwlogs))
