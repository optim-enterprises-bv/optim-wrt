include $(TOPDIR)/rules.mk

PKG_NAME:=libaxl
PKG_RELEASE:=1
PKG_VERSION:=2021-01-26
PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/ASPLes/libaxl
PKG_SOURCE_VERSION:=8430309514d204fde482ade968aa4f64e4637f30
PKG_MIRROR_HASH:=bad353f667ad6a66cee22df4acfdd279b42efe60bd3dc4eb2a6502ace8bf40f9

PKG_LICENSE:=LGPL-2.1-or-later
PKG_LICENSE_FILES:=COPYING

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libaxl
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=LibAxl
  URL:=https://www.aspl.es/xml
endef

define Package/libaxl/description
 LibAxl (or just Axl) is an implementation of the XML 1.0 standard specification.
endef

define Package/libaxl/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libaxl.so* $(1)/usr/lib/
endef

define Build/Configure
	( cd $(PKG_BUILD_DIR) ; ./autogen.sh --disable-py-axl $(CONFIGURE_ARGS) )
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/axl
	$(CP) $(PKG_INSTALL_DIR)/usr/include/axl/*.h $(1)/usr/include/axl
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libaxl.so* $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/axl*.pc $(1)/usr/lib/pkgconfig
endef

$(eval $(call BuildPackage,libaxl))
