include $(TOPDIR)/rules.mk

PKG_NAME:=parodus
PKG_VERSION:=1.1.5
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/xmidt-org/parodus/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=skip

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
  DEPENDS:= +libcurl +mpack +cutils +cjwt
  URL:=https://github.com/xmidt-org/parodus
endef

define Package/parodus/description
  C implementation of the XMiDT client coordinator
endef

define Package/parodus/install

endef

$(eval $(call BuildPackage,parodus))
