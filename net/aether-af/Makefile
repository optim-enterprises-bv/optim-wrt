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
PKG_RELEASE:=4

PKG_MAINTAINER:=Optim Enterprises BV
PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=LICENSE

# Source lives in its own repository so the module can be built, audited and
# redistributed independently of this feed -- which GPL-2.0 requires us to make
# possible for anyone we ship a binary to. Pinned, not tracked: a moving branch
# would change what a tagged firmware contains without changing the feed.
PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/optim-enterprises-bv/aether-af.git
PKG_SOURCE_VERSION:=186f7b699e5a5aab8ed12ea3129c6b325192db92
PKG_MIRROR_HASH:=skip
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)

PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define KernelPackage/aether-af
  SUBMENU:=Network Support
  TITLE:=aether application filtering mechanism
  DEPENDS:=+kmod-nf-conntrack
  FILES:=$(PKG_BUILD_DIR)/src/aether_af.ko
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

# The repository keeps sources under src/, which is also where Kbuild lives, so
# that is what M= points at. Default Build/Prepare unpacks the tarball.
define Build/Compile
	+$(KERNEL_MAKE) M="$(PKG_BUILD_DIR)/src" modules
endef

$(eval $(call KernelPackage,aether-af))
