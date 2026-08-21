
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

#
# luci-app-oaf — built from UPSTREAM source.
#
# Previously vendored 250+ files identical to upstream v6.1.8. Fetching removes
# the redistribution obligation and makes updates a version bump.
#
include $(TOPDIR)/rules.mk

PKG_VERSION:=6.1.8
PKG_RELEASE:=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/destan19/OpenAppFilter.git
PKG_SOURCE_VERSION:=v$(PKG_VERSION)
PKG_SOURCE:=luci-app-oaf-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/luci-app-oaf-$(PKG_VERSION)

LUCI_TITLE:=LuCI support for OAF
LUCI_PKGARCH:=all
LUCI_DEPENDS:=+appfilter +kmod-oaf +luci-compat
PKG_NAME:=luci-app-oaf
PKG_VERSION:=6.1.4
PKG_RELEASE:=1

# Upstream: https://github.com/destan19/OpenAppFilter (GPL-2.0).
# Declared explicitly so the licence propagates into image manifests and
# SBOMs. Integrators build their own firmware from this feed and inherit
# the GPL-2.0 obligations on whatever they then convey; they cannot comply
# with a licence the package never names.
PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Optim Enterprises BV
include $(TOPDIR)/feeds/luci/luci.mk
# call BuildPackage - OpenWrt buildroot signature
