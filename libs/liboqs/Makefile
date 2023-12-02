include $(TOPDIR)/rules.mk

PKG_NAME:=liboqs
PKG_VERSION:=0.9.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=https://codeload.github.com/open-quantum-safe/liboqs/tar.gz/$(PKG_VERSION)?
PKG_HASH:=e6940b4d6dd631e88a2e42f137d12dc59babbd5073751846cabfb4221ece7ad0

PKG_SOURCE_VERSION:=v$(PKG_VERSION)

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

CMAKE_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

CMAKE_OPTIONS += \
		-DOQS_PERMIT_UNSUPPORTED_ARCHITECTURE=ON \
		-DOQS_BUILD_ONLY_LIB=ON

define Package/liboqs
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=C library for prototyping and experimenting with quantum-resistant cryptography
	DEPENDS:=+libopenssl
	URL:=https://openquantumsafe.org/
	MAINTAINER:=HaiBac <mail@bacnh.com>
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/oqs $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/liboqs.* $(1)/usr/lib/
endef

define Package/liboqs/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/liboqs.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,liboqs))