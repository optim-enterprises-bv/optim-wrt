# SPDX-License-Identifier: GPL-3.0-only
#
# Copyright (C) 2023 Facundo Acevedo

include $(TOPDIR)/rules.mk

PKG_NAME:=rosenpass
PKG_VERSION:=0.2.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/rosenpass/rosenpass/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=09dffa62c9bbf3bfa4b34c7667f4052f3c262195a45da9c21c8123f197e7742a

PKG_MAINTAINER:=HaiBac Ngo <mail@bacnh.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENCE

PKG_BUILD_DEPENDS:=rust/host
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include ${TOPDIR}/feeds/packages/lang/rust/rust-package.mk

define Package/rosenpass
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=Rosenpass is a post-quantum-secure VPN that uses WireGuard to transport the actual data.
	DEPENDS:=$(RUST_ARCH_DEPENDS) +libsodium +bash
	URL:=rosenpass.eu
endef

define Package/rosenpass/description
	Rosenpass is a post-quantum-secure VPN that uses WireGuard to transport the actual data.
endef

define Build/Compile
	$(call Build/Compile/Cargo,rosenpass)
endef

define Package/rosenpass/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/bin/* $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/rp $(1)/usr/bin
endef

$(eval $(call RustBinPackage,rosenpass))
$(eval $(call BuildPackage,rosenpass))