include $(TOPDIR)/rules.mk

PKG_NAME:=cimplog
PKG_VERSION:=1.0.3
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/xmidt-org/${PKG_NAME}/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=8c60a72336a7904b132f16dd6a6167fca99efc1af7dabc00d22c0c0df0669c91

PKG_MAINTAINER:=HaiBac <mail@bacnh.com>
PKG_LICENSE:=GPL-3.0-only
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/cimplog
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=Simple logging library for rdklogger
	DEPENDS:=
	URL:=https://github.com/xmidt-org/cimplog
endef

define Package/cimplog/description
	Simple logging library for rdklogger
endef

EXTRA_CFLAGS += -Wno-error=format

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/cimplog
	$(CP) $(PKG_INSTALL_DIR)/usr/include/cimplog/* $(1)/usr/include/cimplog
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcimplog* $(1)/usr/lib
endef

define Package/cimplog/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcimplog* $(1)/usr/lib
endef

$(eval $(call BuildPackage,cimplog))
