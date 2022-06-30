include $(TOPDIR)/rules.mk

PKG_NAME:=ap_config
PKG_VERSION:=2021-08-16
PKG_RELEASE:=4

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=git@github.com:optim-enterprises-bv/APClient.git
PKG_SOURCE_VERSION:=d4fac80c4a2fe6d4dd6ecbf193b9c60d06e14bcc
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/ap_config
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=ap_config
  DEPENDS:=+libaxl +libopenssl +libmariadbclient +libubox +libxml2 +libcurl
endef

define Package/base-files/conffiles
/etc/apconfig/
endef

define Package/ap_config/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/Obj/ac_client $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/ap_config.init $(1)/etc/init.d/ap_config
	install -d -m0700 $(1)/etc/apconfig
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/conf/ac_client.conf $(1)/etc/apconfig
	install -d -m0700 $(1)/etc/apconfig/CA
	$(INSTALL_CONF) ./files/ca.crt ./files/client.{crt,key} $(1)/etc/apconfig/CA
endef

$(eval $(call BuildPackage,ap_config))
