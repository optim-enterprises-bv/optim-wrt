include $(TOPDIR)/rules.mk

PKG_NAME:=wrp-c
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/xmidt-org/wrp-c.git
PKG_MIRROR_HASH:=bf7c875460b8656ffe788a4617c5864b27ac0b72bdb3980f595c51ae3eefccc4
PKG_SOURCE_VERSION:=71f8a39fe39f98da007ed4cdabbb192be1da1685
PKG_HASH:=skip

PKG_MAINTAINER:=HaiBac <mail@bacnh.com>
PKG_LICENSE:=GPL-3.0-only
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/wrp-c
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=C implementation of the Web Routing Protocol
	DEPENDS:=
	URL:=https://github.com/xmidt-org/wrp-c
endef

define Package/wrp-c/description
	C implementation of the Web Routing Protocol
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/wrp-c
	$(CP) $(PKG_INSTALL_DIR)/usr/include/wrp-c/* $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libwrp-c.so* $(1)/usr/lib
endef

define Package/wrp-c/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libwrp-c.so* $(1)/usr/lib
endef

$(eval $(call BuildPackage,wrp-c))
