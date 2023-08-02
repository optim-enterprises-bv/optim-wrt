include $(TOPDIR)/rules.mk

PKG_NAME:=parodus

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/xmidt-org/parodus.git
PKG_SOURCE_VERSION:=1.0.2
PKG_MIRROR_HASH:=d344954a9714bcd0c2d57e265ec70c9c3a59e5d332c777766cc51c6ececb129d

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
