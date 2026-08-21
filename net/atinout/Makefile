#
# atinout — send AT commands to a modem, built from UPSTREAM source.
#
# This directory previously held a raw copy of the upstream project: its own
# hand-written Makefile, atinout.c, README, Changelog, gplv3.txt and all. That is
# not an OpenWrt package — there was no Package/ definition and no BuildPackage
# call, so it could not have built through the feed at all. It also made this
# repository a redistributor of GPL-3.0 source, with the licence and
# change-notice obligations that follow, in exchange for nothing.
#
# Now it is a recipe. The builder fetches the program from its author; this feed
# ships the OpenWrt packaging only.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=atinout
PKG_VERSION:=0.9.1
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/beralt/atinout.git
PKG_SOURCE_VERSION:=4013e8db4cd140c1df24bb90f929efeb9b61b238
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz

PKG_LICENSE:=GPL-3.0-or-later
PKG_LICENSE_FILES:=gplv3.txt
PKG_MAINTAINER:=Optim Enterprises BV

include $(INCLUDE_DIR)/package.mk

define Package/atinout
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Telephony
  TITLE:=Send AT commands to a modem
  URL:=https://github.com/beralt/atinout
endef

define Package/atinout/description
  Reads AT commands from a file or stdin, sends them to a modem and writes the
  responses out. Used by luci-app-atinout-mod.
endef

# Upstream's Makefile builds a single binary and honours CC/CFLAGS/LDFLAGS.
define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS) -DVERSION=\\\"$(PKG_VERSION)\\\"" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		atinout
endef

define Package/atinout/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/atinout $(1)/usr/bin/atinout
endef

$(eval $(call BuildPackage,atinout))
