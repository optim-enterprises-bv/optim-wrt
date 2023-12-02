include $(TOPDIR)/rules.mk

PKG_NAME:=liboqs
PKG_VERSION:=0.7.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=https://codeload.github.com/open-quantum-safe/liboqs/tar.gz/$(PKG_VERSION)?
PKG_HASH:=8432209a3dc7d96af03460fc161676c89e14fca5aaa588a272eb43992b53de76

PKG_SOURCE_VERSION:=v$(PKG_VERSION)

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_DEPENDS:=libubox

CMAKE_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

CMAKE_OPTIONS += -DOQS_PERMIT_UNSUPPORTED_ARCHITECTURE=ON
CMAKE_OPTIONS += -DOQS_BUILD_ONLY_LIB=ON

define Package/liboqs
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=C library for prototyping and experimenting with quantum-resistant cryptography
	DEPENDS:=+libustream-wolfssl
	URL:=https://openquantumsafe.org/
	MAINTAINER:=HaiBac <mail@bacnh.com>
endef

define Package/liboqs/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/liboqs.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,liboqs))