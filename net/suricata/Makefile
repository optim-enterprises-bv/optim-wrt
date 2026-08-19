include $(TOPDIR)/rules.mk

PKG_NAME:=suricata
PKG_VERSION:=8.0.6
PKG_RELEASE:=1

# Official OISF release tarball rather than a git checkout. The previous pin was
# an untagged 2024-03-27 development commit on the 8.0 branch predating the 8.0.0
# release itself, so it carried none of the 8.0.x security fixes.
#
# PKG_HASH below is the sha256 of the signed release artifact, verified twice
# independently, and GPG-verified: "Good signature from Open Information Security
# Foundation (OISF) <releases@openinfosecfoundation.org>", RSA key
# B36FDAF2607E10E8FFA89E5E2BA9C98CCDF1E93A (keys.openpgp.org).
#
# A signed release artifact is stronger provenance than a mirror checksum over a
# tarball we generate ourselves from a clone. It also removes network I/O from
# Build/Configure — see the note there.
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://www.openinfosecfoundation.org/download
PKG_HASH:=b264584edda4a3b2b462050099c7c54f4f35ac0c7164e41084be9b216c090f8c

PKG_CONFIG_DEPENDS:=CONFIG_KERNEL_XDP_SOCKETS

PKG_BUILD_DEPENDS := rust/host python3/host

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/bpf.mk
include $(INCLUDE_DIR)/nls.mk
include ${TOPDIR}/feeds/packages/lang/rust/rust-values.mk

define Package/suricata/config
    source "$(SOURCE)/Config.in"
endef

CONFIGURE_VARS += \
  CARGO_HOME="$(STAGING_DIR)" \
  ac_cv_path_CARGO="$(STAGING_DIR)/host/bin/cargo" \
  ac_cv_path_RUSTC="$(STAGING_DIR)/host/bin/rustc" \

CONFIGURE_ARGS += \
  --target=$(RUSTC_TARGET_ARCH) \
  --host=$(RUSTC_TARGET_ARCH) \
  --build=$(RUSTC_HOST_ARCH) \
  --enable-shared \
  --disable-gccmarch-native \
  --with-gnu-ld \
  --with-sysroot=$(STAGING_DIR)
#	--enable-non-bundled-htp \
#	--with-libhtp-includes=$(STAGING_DIR_HOSTPKG)/include \
#	--with-libhtp-libraries=$(STAGING_DIR_HOSTPKG)/lib
#	--with-sysroot=$(TOOLCHAIN_DIR)

ifeq ($(CONFIG_SURICATA_ENABLE_PYTHON),y)
CONFIGURE_ARGS += --enable-python
endif
ifeq ($(CONFIG_SURICATA_ENABLE_LUAJIT),y)
CONFIGURE_ARGS += --enable-luajit
endif
ifeq ($(CONFIG_SURICATA_ENABLE_GCCPROTECT),y)
CONFIGURE_ARBBBGS += --enable-gccprotect
endif
ifeq ($(CONFIG_SURICATA_ENABLE_GCCPROFILE),y)
CONFIGURE_ARGS += --enable-gccprofile
endif

# For now, x86_64 targets can't use PIE
ifeq ($(CONFIG_PKG_ASLR_PIE_REGULAR),y)
    CONFIGURE_ARGS += --enable-pie
endif

ifeq ($(CONFIG_SURICATA_ENABLE_NFQUEUE),y)
CONFIGURE_ARGS += --enable-nfqueue
endif

ifeq ($(CONFIG_SURICATA_ENABLE_GEOIP),y)
CONFIGURE_ARGS += --enable-geoip
endif

ifeq ($(CONFIG_SURICATA_ENABLE_LIBMAGIC),n)
CONFIGURE_ARGS += --disable-libmagic
endif

ifeq ($(CONFIG_SURICATA_ENABLE_DEBUG),y)
TARGET_CXXFLAGS += -ggdb3
CONFIGURE_ARGS += --enable-debug
endif

ifeq ($(CONFIG_SURICATA_ENABLE_HIREDIS),y)
CONFIGURE_ARGS += --enable-hiredis
endif

ifeq ($(CONFIG_SURICATA_ENABLE_EBPF),y)
CONFIGURE_ARGS += --enable-ebpf --enable-ebpf-build
endif

ifeq ($(CONFIG_SURICATA_ENABLE_NFLOG),y)
CONFIGURE_ARGS += --enable-nflog
endif

# scripts/bundle.sh and autogen.sh were required by the git-checkout flow and are
# both unnecessary against the release tarball, which ships:
#   - rust/vendor/ (9933 vendored cargo entries) — configure detects it and pins
#     cargo offline, so no network fetch mid-compile
#   - a pre-generated ./configure — autogen.sh has nothing to do
#   - suricata-update/ bundled in-tree — bundle.sh has nothing to fetch
# (8.x also dropped libhtp for the Rust htp crate, which is inside rust/vendor.)
#
# Removing them is not just simplification: bundle.sh performed network I/O during
# Build/Configure, which breaks offline and reproducible builds.
define Build/Configure
	( \
		$(CONFIGURE_VARS) cargo install --force --root $(STAGING_DIR)/host cbindgen ; \
		cd $(PKG_BUILD_DIR) && $(CONFIGURE_VARS) ./configure $(CONFIGURE_ARGS) ; \
	)
	$(call Build/Configure/Default)
endef


define Build/Install
	$(call Build/Install/Default,install)
	$(call Build/Install/Default,install-conf)
endef

define Package/suricata
    SUBMENU:=Firewall
    SECTION:=net
    CATEGORY:=Network
    DEPENDS:=@!SMALL_FLASH @!LOW_MEMORY_FOOTPRINT +libexpat +jansson +libelf +libbpf $(BPF_DEPENDS) +libbsd +libpcre2 +libyaml +libpcap +libcap-ng $(ICONV_DEPENDS) $(INTL_DEPENDS) \
        +nspr +libnss +liblz4 +libatomic +libnet-1.2.x +libxdp +libnfnetlink +libunwind +zlib \
	+SURICATA_ENABLE_NFLOG:libnetfilter-log \
  	+SURICATA_ENABLE_NFQUEUE:libnetfilter-queue +SURICATA_ENABLE_NFQUEUE:iptables-mod-nfqueue \
  	+SURICATA_ENABLE_HIREDIS:libhiredis +SURICATA_ENABLE_HIREDIS:libevent2 \
  	+SURICATA_ENABLE_LIBMAGIC:file \
  	+SURICATA_ENABLE_GEOIP:libmaxminddb \
  	+SURICATA_ENABLE_PYTHON:python3 +SURICATA_ENABLE_PYTHON:python3-yaml \
  	+SURICATA_ENABLE_LUAJIT:luajit
    TITLE:=OISF Suricata IDS
    URL:=https://www.openinfosecfoundation.org/
    MENU:=1
endef

define Package/suricata/description
Suricata is an open source-based intrusion detection system (IDS), intrusion
prevention system (IPS), and Network Monitoring System (NMS)
endef

define Package/suricata/conffiles
/etc/config/suricata
/etc/suricata/
endef

define Package/suricata/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(SED) '1c\#!/usr/bin/python3\' -i $(PKG_INSTALL_DIR)/usr/bin/{suricatactl,suricatasc,suricata-update}
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/suricata $(1)/usr/bin/suricata
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/suricatactl $(1)/usr/bin/suricatactl
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/suricatasc $(1)/usr/bin/suricatasc
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/suricata-update $(1)/usr/bin/suricata-update
	
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) -r $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib/

	$(INSTALL_DIR) $(1)/usr/include
	$(CP) -r $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/

	$(INSTALL_DIR) $(1)/etc/suricata
	$(CP) $(PKG_BUILD_DIR)/suricata.yaml \
	$(PKG_BUILD_DIR)/etc/classification.config \
	$(PKG_BUILD_DIR)/threshold.config \
	$(PKG_BUILD_DIR)/etc/reference.config \
	$(1)/etc/suricata/

	$(INSTALL_DIR) $(1)/usr/share/suricata/rules
	$(CP) $(PKG_INSTALL_DIR)/usr/share/suricata/rules/* $(1)/usr/share/suricata/rules/

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/config

	$(INSTALL_BIN) ./files/etc/init.d/suricata $(1)/etc/init.d/suricata
	$(INSTALL_CONF) ./files/etc/config/suricata $(1)/etc/config/suricata
endef

$(eval $(call BuildPackage,suricata))
