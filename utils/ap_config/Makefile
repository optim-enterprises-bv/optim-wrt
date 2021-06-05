include $(TOPDIR)/rules.mk

PKG_NAME:=ap_config
PKG_VERSION:=2021-06-2
PKG_RELEASE:=3

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/optim-enterprises-bv/APClient
PKG_SOURCE_VERSION:=2dbbd62d177e232e884848e59c63b66b1a51e25e

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/ap_config
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=ap_config
  DEPENDS:=+libaxl +libopenssl +libmariadbclient
endef

define Package/base-files/conffiles
/etc/apconfig/
endef

define Package/ap_config/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ac_client $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/ap_config.init $(1)/etc/init.d/ap_config
	install -d -m0700 $(1)/etc/apconfig
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/conf/ac_client.conf $(1)/etc/apconfig
	install -d -m0700 $(1)/etc/apconfig/CA
	$(INSTALL_CONF) ./files/rootCA.crt ./files/client.{crt,key} $(1)/etc/apconfig/CA
endef

$(eval $(call BuildPackage,ap_config))
