include $(TOPDIR)/rules.mk

PKG_NAME:=wrp-c
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/xmidt-org/wrp-c.git
PKG_SOURCE_VERSION:=b5ef4d10cb39905908788bc89ab3e4dab201db8a
PKG_MIRROR_HASH:=232a2d7eb368b472cb6fae5c3a713cd255520db1f9116f48334dccb297d6573a

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

EXTRA_CFLAGS += -Wno-error=stringop-truncation

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
