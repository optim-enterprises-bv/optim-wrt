include $(TOPDIR)/rules.mk

PKG_NAME:=parodus

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/xmidt-org/parodus.git
PKG_SOURCE_VERSION:=85a0d9b71dc1642cd55d88ddb288927f31ab0989
PKG_MIRROR_HASH:=64bcc2d1665bde1e9de5739a1fe3bebe3cd513f058dcad16c2683fcce03ef88a

PKG_MAINTAINER:=HaiBac <mail@bacnh.com>
PKG_LICENSE:=GPL-3.0-only
PKG_LICENSE_FILES:=COPYING

CMAKE_INSTALL:=1

CMAKE_BINARY_SUBDIR:=build

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/parodus
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=C implementation of the XMiDT client coordinator
	DEPENDS:= +libuuid +libssh2
	URL:=https://github.com/xmidt-org/parodus
endef

define Package/parodus/description
	C implementation of the XMiDT client coordinator
endef

define Package/parodus/install	
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/parodus $(1)/usr/bin
endef

$(eval $(call BuildPackage,parodus))
