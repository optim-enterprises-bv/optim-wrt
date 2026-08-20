#
# Open App Filter — built from UPSTREAM source, with a feed-local signature set.
#
# WHY THIS FETCHES INSTEAD OF VENDORING
#
# This package previously carried a full copy of upstream's C source in-tree.
# That copy was byte-identical to upstream v6.1.8 — it contributed nothing except
# a maintenance burden and a redistribution obligation: shipping the sources
# ourselves put this feed squarely under GPL-2.0 §1/§2(a), obliging us to carry
# the licence text and record modifications. Fetching at build time removes that
# entirely. We distribute a recipe; the builder obtains the program from its
# author, exactly as nDPId-testing already does.
#
# It also makes updates a one-line version bump instead of a re-vendor.
#
# WHAT IS STILL OURS, AND MUST NOT BE REPLACED BY UPSTREAM'S COPY
#
# files/feature_en.cfg — the English application signature database.
#   Upstream v6.1.8 ships 38 application signatures. This one carries ~1,349.
#   It is the reason app filtering here recognises the long tail rather than a
#   token sample, and dropping it for upstream's file would silently reduce
#   coverage by ~97% while everything still built and ran. It is installed AFTER
#   upstream's `files/*.cfg`, so it overwrites theirs by design — see the install
#   ordering below, which is load-bearing and not incidental.
#
# files/99-appfilter-guards — uci-defaults setting disable_hnat=1/disable_quic=1.
#   Upstream defaults both to 0. With hardware NAT offload active the accelerated
#   datapath bypasses the classifier, so filtering is configured, reports healthy
#   and inspects nothing; QUIC likewise conceals the SNI that Host-based matching
#   needs. Both defaults produce a filter that appears to work and does not.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=appfilter
PKG_VERSION:=6.1.8
PKG_RELEASE:=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/destan19/OpenAppFilter.git
PKG_SOURCE_VERSION:=v$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Optim Enterprises BV

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

TARGET_CFLAGS += -Werror=implicit-function-declaration

define Package/appfilter
  SECTION:=TT Apps
  CATEGORY:=TT Apps
  DEPENDS:=+libubox +libubus +libuci +libpthread +libjson-c +libblobmsg-json
  TITLE:=OAF client and config service
  URL:=https://github.com/destan19/OpenAppFilter
endef

define Package/appfilter/description
  Open App Filter — DPI-based application filtering for OpenWrt.
  Built from upstream source; ships an extended application signature set.
endef

define Build/Prepare
	$(call Build/Prepare/Default)
	mkdir -p $(PKG_BUILD_DIR)/build
	cp -rf $(PKG_BUILD_DIR)/open-app-filter/src/* $(PKG_BUILD_DIR)/build/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/build/ \
	CC="$(TARGET_CROSS)gcc" \
	CFLAGS="$(TARGET_CFLAGS)" \
	LIBS="$(TARGET_LDFLAGS) -lm -lpthread -lubox -luci -lubus -ljson-c -lblobmsg_json" \
	all
endef

define Package/appfilter/conffiles
/etc/config/appfilter
/etc/config/user_info
endef

define Package/appfilter/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/appfilter
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/uci-defaults

	# Upstream's data files and scripts.
	$(CP) $(PKG_BUILD_DIR)/open-app-filter/files/*.cfg $(1)/etc/appfilter/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/open-app-filter/files/appfilter.init $(1)/etc/init.d/appfilter
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/open-app-filter/files/oaf_rule $(1)/usr/bin/oaf_rule
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/open-app-filter/files/hnat.sh $(1)/usr/bin/hnat.sh
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/open-app-filter/files/gen_class.sh $(1)/usr/bin/gen_class.sh
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/open-app-filter/files/appfilter.config $(1)/etc/config/appfilter
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/open-app-filter/files/user_info.config $(1)/etc/config/user_info
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/build/appfilter $(1)/usr/bin/appfilter

	# ORDER MATTERS. This overwrites upstream's feature_en.cfg, which was copied
	# by the wildcard above. Upstream ships 38 signatures; this ships ~1,349.
	# Moving this line above the wildcard silently reverts coverage to upstream's
	# and nothing else changes — the build succeeds and the filter still runs.
	$(INSTALL_DATA) ./files/feature_en.cfg $(1)/etc/appfilter/feature_en.cfg

	# Guards that must be on for filtering to observe traffic on an accelerated
	# datapath. See the header of this file for why upstream's defaults are unsafe.
	$(INSTALL_BIN) ./files/99-appfilter-guards $(1)/etc/uci-defaults/99-appfilter-guards
endef

$(eval $(call BuildPackage,appfilter))
