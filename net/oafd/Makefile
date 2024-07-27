
include $(TOPDIR)/rules.mk

PKG_NAME:=oafd
PKG_VERSION:=5.0.2
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
include $(INCLUDE_DIR)/package.mk

define Package/oafd
  SECTION:=Derry Apps
  CATEGORY:=Derry Apps
  DEPENDS:=+libubox +libubus  +libuci +libpthread +libjson-c +libblobmsg-json +kmod-oaf
  TITLE:=OpenAppFilter Daemon
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	cp -rf ./src/* $(PKG_BUILD_DIR)
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/ \
	CC="$(TARGET_CROSS)gcc" \
	CFLAGS="$(TARGET_CFLAGS) -g" \
	LIBS="$(TARGET_LDFLAGS) -lm -lpthread -lubox -luci -lubus -ljson-c -lblobmsg_json" \
	all
endef

define Build/Compile/Default

endef

define Package/oafd/description
	OpenAppFilter Daemon
endef


define Package/oafd/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/oafd
	$(INSTALL_DIR) $(1)/etc/config
	$(CP) ./files/*.cfg $(1)/etc/oafd/
	$(INSTALL_BIN) ./files/oafd.init $(1)/etc/init.d/oafd
	$(INSTALL_BIN) ./files/oafd_rule $(1)/usr/bin
	$(INSTALL_BIN) ./files/oafd_gen_class.sh $(1)/usr/bin
	$(INSTALL_BIN) ./files/oafd.config $(1)/etc/config/oafd
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/oafd $(1)/usr/bin
endef

$(eval $(call BuildPackage,oafd))

