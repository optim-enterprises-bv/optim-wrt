#
# Copyright (C) 2026 Optim Enterprises BV
#
# This is free software, licensed under the BSD 3-Clause License.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ac-client
PKG_VERSION:=0.1.0
PKG_RELEASE:=5

# Track optim-enterprises-bv/ac-client over HTTPS at a pinned commit. The
# in-repo package Makefile fetches over an SSH remote
# (git@github.com:optim-enterprises-bv/ac-client.git) and pins the moving
# `main` ref, so it is unbuildable for anyone without a deploy key and is not
# reproducible. Feed builds use HTTPS and an immutable commit.
PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/optim-enterprises-bv/ac-client.git
PKG_SOURCE_VERSION:=3c0191af9d8a7db97f5337340443febaea6050e1
PKG_MIRROR_HASH:=skip
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)

PKG_MAINTAINER:=Optim Enterprises BV
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE

# rust/host builds the Rust toolchain; cmake/host is needed to cross-compile
# aws-lc-sys, pulled in by rustls-post-quantum for the X25519+ML-KEM-768
# hybrid KEM. Both must be declared before package.mk is included --
# rust-package.mk does not add them, and a HOST_BUILD_DEPENDS assignment
# after the includes is inert for a target package.
PKG_BUILD_DEPENDS:=rust/host cmake/host

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(TOPDIR)/feeds/packages/lang/rust/rust-package.mk

define Package/ac-client
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Management
  TITLE:=USP Agent daemon for OpenWrt APs (TR-369 / USP 1.3)
  URL:=https://github.com/optim-enterprises-bv/ac-client
  # RUST_ARCH_DEPENDS gates the package to the architectures rustc can target,
  # so it is not offered (and cannot fail to build) elsewhere. ca-bundle is in
  # DEFAULT_PACKAGES but is declared explicitly because the default ca_file
  # points into it -- without it the agent fails at connect time, not at build
  # time.
  DEPENDS:=$(RUST_ARCH_DEPENDS) +libc +ca-bundle
endef

define Package/ac-client/description
  Rust daemon implementing the TR-369 / USP 1.3 Agent for OpenWrt
  access-point devices. Responsibilities:

    - Device provisioning via certificate exchange
    - Periodic configuration polling and UCI application
    - Firmware upgrade via sysupgrade
    - ValueChange telemetry heartbeats (uptime, load, GPS, wireless, modem)

  TLS uses TLS 1.3 with an X25519+ML-KEM-768 post-quantum hybrid KEM.

  Ships pointed at wss://gw.aether-io.com/usp over the USP WebSocket MTP.
  That endpoint terminates TLS at a public edge which does not currently
  request a client certificate, so the daemon connects and onboards without
  one. Do not treat that as permanent -- client authentication on this
  endpoint is planned, and a device relying on its absence will stop
  connecting when it lands.

  Note that the device is NOT issued a certificate by connecting: the
  controller's Boot! handler does not return one. The bootstrap certificates
  below are installed because ac-client's TLS setup requires a readable
  client cert/key pair to start at all, not because the endpoint uses them.
endef

# Config files and bootstrap certificates survive package upgrades.
define Package/ac-client/conffiles
/etc/apclient/ac_client.conf
/etc/config/optimacs
/etc/apclient/init/ca.crt
/etc/apclient/init/client.crt
/etc/apclient/init/client.key
endef

# The configuration and bootstrap certificates below are held in this feed
# rather than taken from the fetched source tree, so that the deployed
# defaults are reviewable here and do not change silently when
# PKG_SOURCE_VERSION is bumped. They mirror package/ac-client/files/ in
# ac-client at the pinned commit.
define Package/ac-client/install
	# Binary
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/bin/ac-client $(1)/bin/ac-client

	# Flat key=value config (legacy path)
	$(INSTALL_DIR) $(1)/etc/apclient
	$(INSTALL_CONF) ./files/ac_client.conf $(1)/etc/apclient/ac_client.conf

	# UCI config (preferred)
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/etc/config/optimacs $(1)/etc/config/optimacs

	# Bootstrap certificates (CN=00:00:00:00:00:00) for controllers that
	# require client auth before provisioning. The CA private key is not
	# included; it is not needed on the device.
	$(INSTALL_DIR) $(1)/etc/apclient/init
	$(INSTALL_DATA) ./files/init/ca.crt     $(1)/etc/apclient/init/ca.crt
	$(INSTALL_DATA) ./files/init/client.crt $(1)/etc/apclient/init/client.crt
	$(INSTALL_DATA) ./files/init/client.key $(1)/etc/apclient/init/client.key

	# Provisioned certificate directory (populated during CERT exchange)
	$(INSTALL_DIR) $(1)/etc/apclient/certs

	# procd init script
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/ac-client.init $(1)/etc/init.d/ac-client
	#
	# NOTE: /var on OpenWrt is a symlink to /tmp (tmpfs). Do NOT install
	# directories under /var here -- the build system copies the package
	# tree with `cp -fpR` and hits the symlink with EISDIR. The init script
	# creates /var/apclient/{images,firmware} at runtime.
endef

define Package/ac-client/postinst
#!/bin/sh
set -e

echo ""
echo "ac-client installed."
echo ""
echo "Default controller: wss://gw.aether-io.com/usp (USP over WebSocket)."
echo "The endpoint does not currently request a client certificate, so the"
echo "daemon connects and onboards without one. Client authentication on"
echo "this endpoint is planned -- do not rely on its absence."
echo ""
echo "  NOTE: connecting does NOT get this device a certificate. The"
echo "  controller's Boot! handler does not return one. The bootstrap pair"
echo "  in /etc/apclient/init/ exists only because ac-client's TLS setup"
echo "  needs a readable client cert/key to start."
echo ""
echo "Server TLS is verified against /etc/ssl/certs/ca-certificates.crt"
echo "(ca-bundle). ac-client seeds its trust store ONLY from ca_file and"
echo "loads no system roots, so pointing at a private controller means"
echo "repointing ca_file at that controller's CA as well:"
echo ""
echo "   uci set optimacs.agent.server_host='<controller>'"
echo "   uci set optimacs.agent.ws_url='wss://<controller>/usp'"
echo "   uci set optimacs.agent.server_cn='<controller>'"
echo "   uci set optimacs.agent.ca_file='/etc/apclient/init/ca.crt'"
echo "   uci commit optimacs"
echo ""
echo "  WARNING: the bootstrap certificates in /etc/apclient/init/ are"
echo "  public -- they are shipped in this package. Replace them for any"
echo "  deployment that authenticates clients by certificate."
echo ""
echo "View current config:  uci show optimacs.agent"
echo ""
echo "Then enable and start:"
echo "  /etc/init.d/ac-client enable"
echo "  /etc/init.d/ac-client start"
echo ""
endef

$(eval $(call RustBinPackage,ac-client))
$(eval $(call BuildPackage,ac-client))
