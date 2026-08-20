
#
# OAF kernel DPI driver — built from UPSTREAM source.
#
# Previously vendored a full copy of upstream's C source in-tree, byte-identical
# to v6.1.8. That added no value and made this feed a redistributor of GPL-2.0
# code, with the licence and change-notice obligations that follow. Fetching at
# build time removes both: we ship a recipe, the builder obtains the program from
# its author.
#
# The build-system overrides below are ours and must survive version bumps — see
# the comments on each.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=oaf
PKG_VERSION:=6.1.8
PKG_RELEASE:=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/destan19/OpenAppFilter.git
PKG_SOURCE_VERSION:=v$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Optim Enterprises BV

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

PKG_AUTOLOAD:=oaf
RSTRIP:=:

define KernelPackage/oaf
  SECTION:=TT Apps
  CATEGORY:=TT Apps
  TITLE:=OAF kernel DPI driver
  URL:=https://github.com/destan19/OpenAppFilter
  FILES:=$(PKG_BUILD_DIR)/oaf.ko 
  DEPENDS:=+kmod-ipt-conntrack
  KCONFIG:=
  # AUTOLOAD:=$(call AutoLoad,0,$(PKG_AUTOLOAD))
endef

define KernelPackage/oaf/description
  open appfilter kernel module 
endef


EXTRA_CFLAGS:=-Wno-declaration-after-statement -Wno-strict-prototypes -Wno-unused-variable -Wno-implicit-fallthrough -Wno-missing-braces -Wno-parentheses -Wno-format -Wno-missing-prototypes -Wno-missing-declarations



MAKE_OPTS:= \
	$(KERNEL_MAKE_FLAGS) \
	M="$(PKG_BUILD_DIR)" \
	EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
	$(EXTRA_KCONFIG)

# Source now arrives from upstream's tarball, where the module lives under oaf/src.
# The explicit copy is still required: 2016-era buildsystems lack the modern
# ./src auto-copy in Build/Prepare/Default, so removing this breaks the QSDK
# targets even though it looks redundant on a current tree.
define Build/Prepare
	$(call Build/Prepare/Default)
	$(CP) $(PKG_BUILD_DIR)/oaf/src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		ARCH="$(LINUX_KARCH)" \
		M="$(PKG_BUILD_DIR)" \
		EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
		$(EXTRA_KCONFIG) \
		modules
endef

$(eval $(call KernelPackage,oaf))

