
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=oaf
include $(INCLUDE_DIR)/package.mk

PKG_AUTOLOAD:=oaf
RSTRIP:=:

define KernelPackage/oaf
  SECTION:=TT Apps
  CATEGORY:=TT Apps
  TITLE:=OAF kernel DPI driver
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

# 2016-era buildsystems lack the modern ./src auto-copy in Build/Prepare/Default
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
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

