include $(TOPDIR)/rules.mk

PKG_NAME:=wrp-c
PKG_VERSION:=2.0.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/xmidt-org/${PKG_NAME}/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=skip

PKG_MAINTAINER:=HaiBac <mail@bacnh.com>
PKG_LICENSE:=GPL-3.0-only
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/meson.mk

define Package/wrp-c
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=C implementation of the Web Routing Protocol
	DEPENDS:= +cJSON +libopenssl +trower-base64
	URL:=https://github.com/xmidt-org/cjwt
endef

define Package/wrp-c/description
	C implementation of the Web Routing Protocol
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/cjwt
	$(CP) $(PKG_INSTALL_DIR)/usr/include/cjwt/* $(1)/usr/include/cjwt
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcjwt.so* $(1)/usr/lib
endef

define Package/wrp-c/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcjwt.so* $(1)/usr/lib
endef

$(eval $(call BuildPackage,wrp-c))
