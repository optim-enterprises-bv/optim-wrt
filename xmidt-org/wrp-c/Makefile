include $(TOPDIR)/rules.mk

PKG_NAME:=wrp-c
PKG_VERSION:=2.0.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/xmidt-org/${PKG_NAME}/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=7ad78cd7c43a7fec8858e2bc1367daffd081781db8e7726fe99a7aceac500942

PKG_MAINTAINER:=HaiBac <mail@bacnh.com>
PKG_LICENSE:=GPL-3.0-only
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/meson.mk

define Package/wrp-c
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=C implementation of the Web Routing Protocol
	DEPENDS:= +cutils +ludocode-mpack
	URL:=https://github.com/xmidt-org/cjwt
endef

define Package/ludocode-mpack
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=MPack - A C encoder/decoder for the MessagePack serialization format / msgpack.org[C]
	DEPENDS:=
	URL:=https://github.com/ludocode/mpack
endef

define Package/wrp-c/description
	C implementation of the Web Routing Protocol
endef

define Package/libmpack/description
	MPack - A C encoder/decoder for the MessagePack serialization format / msgpack.org[C]
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/wrp-c
	$(CP) $(PKG_INSTALL_DIR)/usr/include/wrp-c/* $(1)/usr/include/wrp-c
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libmpack.so* $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libwrp-c.so* $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/mpack.pc $(1)/usr/lib/pkgconfig
endef

define Package/ludocode-mpack/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libmpack.so* $(1)/usr/lib
endef

define Package/wrp-c/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libwrp-c.so* $(1)/usr/lib
endef

$(eval $(call BuildPackage,wrp-c))
$(eval $(call BuildPackage,ludocode-mpack))

