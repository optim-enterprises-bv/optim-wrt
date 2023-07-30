include $(TOPDIR)/rules.mk

PKG_NAME:=libcutils
PKG_VERSION:=2.1.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/xmidt-org/cutils/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=skip

PKG_MAINTAINER:=HaiBac <mail@bacnh.com>
PKG_LICENSE:=GPL-3.0-only
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/meson.mk

define Package/libcutils
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Common functions used in the xmidt-org programs.
  DEPENDS:= +libcunit
  URL:=https://github.com/xmidt-org/cutils
endef

define Package/libcutils/description
  C implementation of the XMiDT client coordinator
endef

define Build/InstallDev

endef

define Package/libcutils/install
	$(INSTALL_DIR) $(1)/usr/lib

endef

$(eval $(call BuildPackage,libcutils))
