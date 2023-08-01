include $(TOPDIR)/rules.mk

PKG_NAME:=trower-base64
PKG_VERSION:=1.1.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/xmidt-org/trower-base64/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=2ba31fcb81bdb2a7568e4f814a50d10c262e3287e5a6565484ffdafe0980f40b

PKG_MAINTAINER:=HaiBac <mail@bacnh.com>
PKG_LICENSE:=GPL-3.0-only
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/trower-base64
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=C implementation of base64 encode/decode
	DEPENDS:=
	URL:=https://github.com/xmidt-org/trower-base64
endef

define Package/trower-base64/description
	C implementation of base64 encode/decode
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/trower-base64/* $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libtrower-base64.so* $(1)/usr/lib
endef

define Package/trower-base64/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libtrower-base64.so* $(1)/usr/lib
endef

$(eval $(call BuildPackage,trower-base64))
