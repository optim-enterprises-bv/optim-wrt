# SPDX-License-Identifier: GPL-3.0-only
#
# Copyright (C) 2021 ImmortalWrt.org

include $(TOPDIR)/rules.mk

PKG_NAME:=msgpack
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/msgpack/msgpack-c.git
PKG_SOURCE_VERSION:=cpp-3.1.1
PKG_MIRROR_HASH:=ebafeaf966b42495c1510dff0df71076bd87728cf9de951c25406b3a3c03c198

PKG_LICENSE:=BSL-1.0
PKG_LICENSE_FILES:=COPYING LICENSE_1_0.txt
PKG_MAINTAINER:=Tianling Shen <cnsztl@immortalwrt.org>

PKG_BUILD_PARALLEL:=1
CMAKE_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

CMAKE_OPTIONS+= \
	-DBUILD_SHARED_LIBS=ON \
	-DMSGPACK_BUILD_EXAMPLES=OFF \
	-DMSGPACK_BUILD_TESTS=OFF \
	-DMSGPACK_ENABLE_STATIC=OFF \
	-DMSGPACK_ENABLE_CXX=OFF

define Package/msgpack
	SECTION:=lib
	CATEGORY:=Libraries
	TITLE:=MessagePack implementation for C
	URL:=https://github.com/msgpack/msgpack-c
endef

define Package/msgpack/description
	MessagePack is an efficient binary serialization format, which lets
	you exchange data among multiple languages like JSON, except that
	it's faster and smaller. Small integers are encoded into a single
	byte and short strings require only one extra byte in addition to
	the strings themselves.
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/msgpack
	$(CP) $(PKG_INSTALL_DIR)/usr/include/msgpack* $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libmsgpackc.so* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/msgpack.pc $(1)/usr/lib/pkgconfig/
endef

define Package/msgpack/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libmsgpackc.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,msgpack))
