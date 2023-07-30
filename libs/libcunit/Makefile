# 
# Copyright (C) 2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk

PKG_NAME:=libcunit
PKG_VERSION:=2.1-3
PKG_RELEASE:=1

PKG_SOURCE:=CUnit-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=@SF/cunit
PKG_HASH:=f5b29137f845bb08b77ec60584fdb728b4e58f1023e6f249a464efa49a40f214
PKG_BUILD_DIR:=$(BUILD_DIR)/CUnit-$(PKG_VERSION)

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libcunit
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Unit testing framework for C
  URL:=http://cunit.sourceforge.net/
endef

define Package/libcunit/description
  CUnit, lightweight unit testing for C
  CUnit is a lightweight system for writing, administering, and running unit
  tests in C.  It provides C programmers a basic testing functionality with a
  flexible variety of user interfaces.
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/CUnit
	$(INSTALL_DATA)	$(PKG_INSTALL_DIR)/usr/include/CUnit/* $(1)/usr/include/CUnit
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcunit.* $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/cunit.pc $(1)/usr/lib/pkgconfig
endef

define Package/libcunit/install
	$(INSTALL_DIR)	$(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcunit.* $(1)/usr/lib
endef

$(eval $(call BuildPackage,libcunit))