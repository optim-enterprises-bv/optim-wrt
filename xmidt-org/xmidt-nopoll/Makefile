#
# libnopoll - Makefile for noPoll C WebSocket library
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=xmidt-nopoll
PKG_VERSION:=1.0.4
PKG_RELEASE:=2

PKG_SOURCE_URL:=https://codeload.github.com/xmidt-org/nopoll/tar.gz/$(PKG_VERSION)?
PKG_SOURCE:=nopoll-$(PKG_VERSION).tar.gz
PKG_HASH:=daa8f8b6eb99210df5a0323bdbab74bfc73700126a47ae47c2bcd21ee6405abb
PKG_BUILD_DIR:=$(BUILD_DIR)/nopoll-$(PKG_VERSION)

PKG_MAINTAINER:=John Clark <inindev@gmail.com>
PKG_LICENSE:=LGPL-2.1
PKG_LICENSE_FILES:=COPYING

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/xmidt-nopoll
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libopenssl
  TITLE:=noPoll C WebSocket Library
  URL:=https://github.com/xmidt-org/nopoll
endef

define Package/xmidt-nopoll/description
  This package contains the noPoll C WebSocket library.
endef

TARGET_LDFLAGS += -lcrypto -lpthread

CONFIGURE_ARGS += \
	--enable-nopoll-log=no \
	--enable-nopoll-doc=no

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/{lib,include}
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnopoll.{a,so*} $(1)/usr/lib/
endef

define Package/xmidt-nopoll/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnopoll.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,xmidt-nopoll))