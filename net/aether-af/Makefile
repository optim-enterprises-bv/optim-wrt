#
# Copyright (C) 2026 Optim Enterprises BV
#
# The MODULE is GPL-2.0 -- see src/af_module.c. That obligation is confined to
# the mechanism: it holds hashes, not signatures, and every decision it
# enforces is made by aether-sensord (BSD-3-Clause) and pushed over netlink.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=aether-af
PKG_VERSION:=0.1.0
PKG_RELEASE:=1

PKG_MAINTAINER:=Optim Enterprises BV
PKG_LICENSE:=GPL-2.0-only

PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define KernelPackage/aether-af
  SUBMENU:=Network Support
  TITLE:=aether application filtering mechanism
  DEPENDS:=+kmod-nf-conntrack
  FILES:=$(PKG_BUILD_DIR)/aether_af.ko
  AUTOLOAD:=$(call AutoLoad,50,aether_af)
endef

define KernelPackage/aether-af/description
  In-kernel mechanism for aether application filtering.

  Takes packets from a netfilter FORWARD hook, extracts a TLS SNI or HTTP Host,
  hashes it, looks the hash up in a table it was given, and drops or accepts.

  It holds NO signatures, NO application names, NO categories, NO schedules and
  NO taxonomy. It cannot tell you what YouTube is. Every decision it enforces
  is made in userspace by aether-sensord and compiled to (hash, subject,
  verdict) triples before crossing the netlink boundary. That separation is
  what confines this module's GPL obligation to the mechanism.

  Blocks by SILENT DROP. There is deliberately no TCP RST: nf_send_reset() is
  the one GPL-only symbol Open App Filter depends on, and the price of doing
  without it is that a blocked connection hangs rather than failing fast.

  Loading this module blocks nothing on its own. It has no rules until the
  daemon sends them.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	+$(KERNEL_MAKE) M="$(PKG_BUILD_DIR)" modules
endef

$(eval $(call KernelPackage,aether-af))
