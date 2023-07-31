include $(TOPDIR)/rules.mk

PKG_NAME:=cjwt
PKG_VERSION:=2.1.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/xmidt-org/${PKG_NAME}/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=skip

PKG_MAINTAINER:=HaiBac <mail@bacnh.com>
PKG_LICENSE:=GPL-3.0-only
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/meson.mk

define Package/cjwt
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=A c jwt implementation
	DEPENDS:= +cJSON +libopenssl +trower-base64
	URL:=https://github.com/xmidt-org/cjwt
endef

define Package/cjwt/description
	A c jwt implementation
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/cjwt
	$(CP) $(PKG_INSTALL_DIR)/usr/include/cjwt/* $(1)/usr/include/cjwt
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcjwt.so* $(1)/usr/lib
endef

define Package/cjwt/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcjwt.so* $(1)/usr/lib
endef

$(eval $(call BuildPackage,cjwt))
