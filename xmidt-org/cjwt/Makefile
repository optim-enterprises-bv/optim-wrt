include $(TOPDIR)/rules.mk

PKG_NAME:=cjwt
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/xmidt-org/cjwt.git
PKG_SOURCE_VERSION:=1b023c41bb2d6dbbf493c202ed81f06c84d5b51b
PKG_HASH:=skip

PKG_MAINTAINER:=HaiBac <mail@bacnh.com>
PKG_LICENSE:=GPL-3.0-only
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

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

CMAKE_OPTIONS += \
	-DBUILD_TESTING=OFF

EXTRA_CFLAGS += -Wno-error=deprecated-declarations

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
