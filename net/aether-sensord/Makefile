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
endef

define Package/aether-sigtool/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/aether-sigtool $(1)/usr/bin/aether-sigtool
endef

$(eval $(call BuildPackage,aether-sigtool))
