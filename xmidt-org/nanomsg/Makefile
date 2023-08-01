include $(TOPDIR)/rules.mk

PKG_NAME:=nanomsg
PKG_VERSION:=1.1.4
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/nanomsg/${PKG_NAME}/tar.gz/$(PKG_VERSION)?
PKG_HASH:=ae5c688e874bf9bf1db764d08538c54f6ce9e4dc78bc5948008a304d519ad3b6

PKG_MAINTAINER:=HaiBac <mail@bacnh.com>
PKG_LICENSE:=GPL-3.0-only
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/nanomsg
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=nanomsg library
	DEPENDS:=
	URL:=https://github.com/nanomsg/nanomsg
endef

define Package/nanomsg/description
	nanomsg library
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/nanomsg
	$(CP) $(PKG_INSTALL_DIR)/usr/include/nanomsg/* $(1)/usr/include/nanomsg
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnanomsg.so* $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/nanomsg.pc $(1)/usr/lib/pkgconfig
endef

define Package/nanomsg/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnanomsg.so* $(1)/usr/lib
endef

$(eval $(call BuildPackage,nanomsg))
