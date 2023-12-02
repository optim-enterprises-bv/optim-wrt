include $(TOPDIR)/rules.mk

PKG_NAME:=oqs-provider
PKG_VERSION:=0.5.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=https://codeload.github.com/open-quantum-safe/oqs-provider/tar.gz/$(PKG_VERSION)?
PKG_HASH:=2b39ff8ec4eb52a2ef611910baa2286c542b3b9ec103d11832323182502e41bc

PKG_SOURCE_VERSION:=v$(PKG_VERSION)

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

CMAKE_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/oqs-provider
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=OpenSSL 3 provider containing post-quantum algorithms
	DEPENDS:=+libopenssl +liboqs
	URL:=https://openquantumsafe.org/
	MAINTAINER:=HaiBac <mail@bacnh.com>
endef

define Package/oqs-provider/install
	$(INSTALL_DIR) $(1)/usr/lib/ossl-modules
	$(CP) $(PKG_BUILD_DIR)/lib/oqsprovider.so $(1)/usr/lib/ossl-modules/
endef

$(eval $(call BuildPackage,oqs-provider))
