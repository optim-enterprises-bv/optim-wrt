include $(TOPDIR)/rules.mk

PKG_NAME:=usteer-ng
PKG_VERSION:=1.0.2
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/NilsRo/usteer-ng.git
PKG_SOURCE_VERSION:=8fc9fa95dd3133d392068d5c5e592cb9e069f6c9
PKG_SOURCE_DATE:=2026-09-08
PKG_MIRROR_HASH:=skip
PKG_HASH:=skip

PKG_MAINTAINER:=Nils Hendrik Rottgardt <n.rottgardt@gmail.com>
PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_DEPENDS:=libpcap
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/usteer-ng
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Wireless
  DEPENDS:=+libubox +libubus +libblobmsg-json +libnl-tiny
  TITLE:=Usteer - Next Generation OpenWrt AP roaming assist daemon
  PROVIDES:=usteer $(PKG_NAME)
  REPLACES:=usteer
  URL:=https://github.com/NilsRo/usteer-ng/
endef

define Package/usteer-ng/conffiles
/etc/config/usteer
endef

define Package/usteer-ng/install
	$(INSTALL_DIR) $(1)/sbin $(1)/etc/init.d $(1)/etc/config
	$(CP) ./files/* $(1)/
	$(CP) $(PKG_BUILD_DIR)/usteerd $(1)/sbin/
endef

$(eval $(call BuildPackage,usteer-ng))
