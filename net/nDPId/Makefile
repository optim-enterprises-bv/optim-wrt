include $(TOPDIR)/rules.mk

PKG_NAME:=nDPId
PKG_VERSION:=1.5
PKG_RELEASE:=$(AUTORELEASE)

#PKG_SOURCE:=$(PKG_VERSION).tar.gz
#PKG_SOURCE_URL:=https://github.com/utoni/nDPId/archive/refs/tags
#PKG_HASH:=a512d5fbdcdaef01a0c3ee7f6b8870ac9c6c870e11c1a30b782550e2cd247de7

# 1.5 Release does not work with current libndpi builds.
PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/utoni/nDPId.git
PKG_SOURCE_VERSION:=c9514136b7c4246a57b85474d1a8e376a9009d4a
PKG_HASH:=aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)

PKG_MAINTAINER:=Toni
PKG_LICENSE:=GPL-3
PKG_LICENSE_FILES:=COPYING

CMAKE_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/nDPId
 TITLE:=nDPId is a tiny nDPI based daemons / toolkit
 SECTION:=net
 CATEGORY:=Network
 DEPENDS:=@!SMALL_FLASH @!LOW_MEMORY_FOOTPRINT +libpcap +LIBNDPI_GCRYPT:libgcrypt +libndpi
 URL:=http://github.com/utoni/nDPId
endef

define Package/nDPId/description
  nDPId is a set of daemons and tools to capture, process and classify network flows.
  It's only dependencies (besides a half-way modern c library and POSIX threads) are libnDPI (>= 3.6.0 or current github dev branch) and libpcap.
endef

define Package/nDPId/config
config NDPID_COLLECTD_SUPPORT
	bool "nDPId collectd support"
	depends on PACKAGE_nDPId && PACKAGE_collectd-mod-exec
	default n
	help
	  This option enables collectd to gather nDPId statistics via plugin-exec.
	  Disabled by default.
endef

CMAKE_OPTIONS += -DBUILD_EXAMPLES=ON
CMAKE_OPTIONS += -DBUILD_NDPI=OFF
CMAKE_OPTIONS += -DENABLE_SANITIZER=OFF
CMAKE_OPTIONS += -DENABLE_MEMORY_PROFILING=OFF

ifneq ($(CONFIG_LIBNDPI_GCRYPT),)
CMAKE_OPTIONS += -DNDPI_WIDTH_GCRYPT=ON
endif

ifdef NDPID_COLLECTD_SUPPORT
define Package/nDPId/install-collectd-files
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/nDPIsrvd-collectd $(1)/usr/bin/
endef
endif

define Package/nDPId/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/nDPId $(1)/usr/sbin/

	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/nDPId-test $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/nDPIsrvd $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/nDPIsrvd-captured $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/nDPIsrvd-collectd $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/nDPIsrvd-json-dump $(1)/usr/bin/

	$(call Package/nDPId/install-collectd-files)
endef

$(eval $(call BuildPackage,nDPId))
