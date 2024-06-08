include $(TOPDIR)/rules.mk

PKG_NAME:=nDPId-master
PKG_VERSION:=1.5
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=git@github.com:outbackdingo/nDPId.git
PKG_SOURCE_VERSION:=7969cd632f0357adebf77aebcd2764a525e609ce
PKG_HASH:=aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)

PKG_MAINTAINER:=Toni
PKG_LICENSE:=GPL-3
PKG_LICENSE_FILES:=COPYING

CMAKE_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/nDPId-master
 TITLE:=nDPId is a tiny nDPI based daemons / toolkit (master branch)
 SECTION:=net
 CATEGORY:=Network
 DEPENDS:=@!SMALL_FLASH @!LOW_MEMORY_FOOTPRINT +libpcap +zlib +LIBNDPI_GCRYPT:libgcrypt
 URL:=http://github.com/utoni/nDPId
endef

define Package/nDPId-master/description
  nDPId is a set of daemons and tools to capture, process and classify network flows.
  It's only dependencies (besides a half-way modern c library and POSIX threads) are libnDPI (>= 3.6.0 or current github dev branch) and libpcap.
endef

define Package/nDPId-master/config
config NDPID_MASTER_COLLECTD_SUPPORT
   bool "nDPId collectd support"
   depends on PACKAGE_nDPId-master && PACKAGE_collectd-mod-exec && !CONFIG_NDPID_COLLECTD_SUPPORT
   default n
   help
     This option enables collectd to gather nDPId statistics via plugin-exec.
     Disabled by default.
endef

CMAKE_OPTIONS += -DBUILD_EXAMPLES=ON
CMAKE_OPTIONS += -DBUILD_NDPI=OFF
CMAKE_OPTIONS += -DNDPI_NO_PKGCONFIG=ON
CMAKE_OPTIONS += -DENABLE_ZLIB=ON
CMAKE_OPTIONS += -DENABLE_SANITIZER=OFF
CMAKE_OPTIONS += -DENABLE_MEMORY_PROFILING=OFF
CMAKE_OPTIONS += -DSTATIC_LIBNDPI_INSTALLDIR="$(PKG_BUILD_DIR)/libnDPI/install"

TARGET_CFLAGS += -DLIBNDPI_STATIC=1

ifneq ($(CONFIG_LIBNDPI_GCRYPT),)
CMAKE_OPTIONS += -DNDPI_WIDTH_GCRYPT=ON
endif

ifdef NDPID_MASTER_COLLECTD_SUPPORT
define Package/nDPId-master/install-collectd-files
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/nDPIsrvd-collectd $(1)/usr/bin/nDPIsrvd-master-collectd
endef
endif

define Build/Prepare
	$(call Build/Prepare/Default)
	$(PKG_UNPACK)
	$(Build/Patch)
	env \
		CC="$(TARGET_CC)" \
		AR="$(TARGET_AR)" \
		RANLIB="$(TARGET_RANLIB)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		CPPFLAGS="$(TARGET_CPPFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		MAKE_PROGRAM="$(MAKE)" \
		$(PKG_BUILD_DIR)/scripts/get-and-build-libndpi.sh
endef

define Package/nDPId-master/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/nDPId $(1)/usr/sbin/nDPId-master

	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/nDPId-test $(1)/usr/bin/nDPId-master-test
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/nDPIsrvd $(1)/usr/bin/nDPIsrvd-master
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/nDPIsrvd-captured $(1)/usr/bin/nDPIsrvd-master-captured
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/nDPIsrvd-collectd $(1)/usr/bin/nDPIsrvd-master-collectd
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/nDPIsrvd-json-dump $(1)/usr/bin/nDPIsrvd-master-json-dump

	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_BIN) $(PKG_NAME).init $(1)/etc/init.d/$(PKG_NAME)
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) $(PKG_NAME).config $(1)/etc/config/$(PKG_NAME)

	$(call Package/nDPId-master/install-collectd-files)
endef

$(eval $(call BuildPackage,nDPId-master))
