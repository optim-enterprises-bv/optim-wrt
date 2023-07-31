include $(TOPDIR)/rules.mk

PKG_NAME:=cutils
PKG_VERSION:=2.1.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/xmidt-org/cutils/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=d8a6fea1cc8931b513d047bacf5f39525c30bc47b1741469e52bcf5607d1b0db

PKG_MAINTAINER:=HaiBac <mail@bacnh.com>
PKG_LICENSE:=GPL-3.0-only
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/meson.mk

define Package/cutils
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=Common functions used in the xmidt-org programs.
	DEPENDS:= +libcunit
	URL:=https://github.com/xmidt-org/cutils
endef

define Package/cutils/description
	C implementation of the XMiDT client coordinator
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/cutils
	$(CP) $(PKG_INSTALL_DIR)/usr/include/cutils/* $(1)/usr/include/cutils
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcutils.so* $(1)/usr/lib/cutils
endef

define Package/cutils/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcutils.so* $(1)/usr/lib
endef

$(eval $(call BuildPackage,cutils))
