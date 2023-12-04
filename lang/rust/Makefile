# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
include $(TOPDIR)/rules.mk

PKG_NAME:=rust
PKG_VERSION:=1.59.0
PKG_RELEASE:=1

PKG_LICENSE:=Apache-2.0 MIT
PKG_LICENSE_FILES:=LICENSE-APACHE LICENSE-MIT

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/rust-lang/rust.git
PKG_SOURCE_DATE:=2021-12-02
PKG_SOURCE_VERSION:=db9d1b20bba1968c1ec1fc49616d4742c1725b4b
PKG_MIRROR_HASH:=187ee9de9f8f39f582b808648377bd856a71a02f869fb3035ecb12d9db20be10

PKG_HOST_ONLY:=1
PKG_BUILD_PARALLEL:=1

HOST_USE_NINJA:=1
CMAKE_INSTALL:=1

include $(INCLUDE_DIR)/cmake.mk
include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk

# Requires Python3 to build
HOST_BUILD_DEPENDS:=python3/host

# rust environmental variables
include ./rust_environment.mk

# Rust Temp Directory
RUST_TMP_DIR:=$(TMP_DIR)/rust-install

RUST_INSTALL_HOST_FILENAME:=$(PKG_NAME)-$(PKG_VERSION)-$(RUSTC_HOST_ARCH)-install.tar.xz
RUST_INSTALL_TARGET_FILENAME:=$(PKG_NAME)-$(PKG_VERSION)-$(RUSTC_TARGET_ARCH)-install.tar.xz

# Rust-lang has an uninstall script
RUST_UNINSTALL:=$(CARGO_HOME)/lib/rustlib/uninstall.sh

# This checks to see if the dist installation already exists
IS_RUSTC_INSTALLED:=$(or $(and $(wildcard $(CARGO_HOME)/bin/rustc),true),false)
IS_CARGO_INSTALLED:=$(or $(and $(wildcard $(CARGO_HOME)/bin/cargo),true),false)

# MUSL Target Flags
MUSL_CONFIGURE_ARGS = \
  --set=target.$(RUSTC_TARGET_ARCH).ar=$(TARGET_AR) \
  --set=target.$(RUSTC_TARGET_ARCH).cc=$(TARGET_CC_NOCACHE) \
  --set=target.$(RUSTC_TARGET_ARCH).cxx=$(TARGET_CXX_NOCACHE) \
  --set=target.$(RUSTC_TARGET_ARCH).linker=$(TARGET_CC_NOCACHE) \
  --set=target.$(RUSTC_TARGET_ARCH).musl-root=$(TOOLCHAIN_DIR) \
  --set=target.$(RUSTC_TARGET_ARCH).ranlib=$(TARGET_RANLIB)

# CARGO_HOME is an environmental
HOST_CONFIGURE_OPTS += CARGO_HOME="$(CARGO_HOME)"

# The Flags are Common between HOST and TARGET
RUST_COMMON_ARGS := \
  --build=$(RUSTC_HOST_ARCH) \
  --dist-compression-formats=xz \
  --enable-llvm-link-shared \
  --enable-llvm-plugins \
  --enable-missing-tools \
  --enable-ninja \
  --enable-optimize \
  --enable-optimize-llvm \
  --enable-parallel-compiler \
  --enable-sanitizers \
  --release-channel=nightly \
  ${MUSL_CONFIGURE_ARGS}

##
# Because OpenWrt uses default configure flags that are not recognized
# we override the default and set our own.
#
# Rust HOST Configuration Arguments
HOST_CONFIGURE_ARGS = \
  --target=$(RUSTC_TARGET_ARCH),$(RUSTC_HOST_ARCH) \
  --host=$(RUSTC_HOST_ARCH) \
  --prefix=$(CARGO_HOME) \
  --bindir=$(CARGO_HOME)/bin \
  --libdir=$(CARGO_HOME)/lib \
  --sysconfdir=$(CARGO_HOME)/etc \
  --datadir=$(CARGO_HOME)/share \
  --mandir=$(CARGO_HOME)/man \
  ${RUST_COMMON_ARGS}

# TODO: For future use
# Rust TARGET Configuration Args
#CONFIGURE_ARGS = \
#  --target=$(RUSTC_TARGET_ARCH) \
#  --host=$(RUSTC_TARGET_ARCH) \
#  --prefix=$(CONFIGURE_PREFIX) \
#  --bindir=$(CONFIGURE_PREFIX)/bin \
#  --libdir=$(CONFIGURE_PREFIX)/lib \
#  --sysconfdir=$(CONFIGURE_PREFIX)/etc \
#  --datadir=$(CONFIGURE_PREFIX)/share \
#  --mandir=$(CONFIGURE_PREFIX)/man \
#  $(RUST_COMMOM_ARGS) \
#  --set=llvm.cxxflags='$(RUST_LLVM_CXXFLAGS)'

# Add rust_config.mk Debug flags
#include ./rust_config.mk

define Host/Prepare
	# Ensure rust temp directory
	[ -d $(RUST_TMP_DIR) ] || \
	  mkdir -p $(RUST_TMP_DIR)

	$(call Host/Prepare/Default)
endef

define Host/Uninstall
	# Call the Uninstall script
	[ -f $(RUST_UNINSTALL) ] && \
	  $(BASH) $(RUST_UNINSTALL) || echo No Uninstall

	rm -rf $(RUST_TMP_DIR)
endef

# Makes and then packages the dist artifacts
define Host/Compile
	cd $(HOST_BUILD_DIR) && \
	  $(PYTHON) x.py --config ./config.toml dist build-manifest cargo llvm-tools \
	    rustc rust-std rust-src

	$(call Host/PackageDist)
endef

# Distribution Artifacts are packaged by Host/PackageDist
# We just need to extract and install
define Host/Install
	$(TAR) -C $(RUST_TMP_DIR) -xJf $(DL_DIR)/$(RUST_INSTALL_HOST_FILENAME) && \
	$(TAR) -C $(RUST_TMP_DIR) -xJf $(DL_DIR)/$(RUST_INSTALL_TARGET_FILENAME)

	cd $(RUST_TMP_DIR) && \
	  find -iname "*.xz" -exec tar -xJf {} ";" && \
	  find ./* -type f -name install.sh -execdir sh {} --prefix=$(CARGO_HOME) --disable-ldconfig \;
endef

# Packages the Distribution Artifacts into HOST and TARGET bundles.
define Host/PackageDist
	cd $(HOST_BUILD_DIR)/build/dist && \
	  $(TAR) -cJf $(DL_DIR)/$(RUST_INSTALL_TARGET_FILENAME) \
	  rust-*-$(RUSTC_TARGET_ARCH).tar.xz

	cd $(HOST_BUILD_DIR)/build/dist && \
	  $(TAR) -cJf $(DL_DIR)/$(RUST_INSTALL_HOST_FILENAME) \
	  --exclude rust-*-$(RUSTC_TARGET_ARCH).tar.xz *.xz
endef

define Package/rust
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Rust Programming Language Compiler
  URL:=https://www.rust-lang.org/
  DEPENDS:=@(aarch64||arm||mips64||mips||mipsel||x86_64) \
    +pkg-config +libunwind
endef

define Package/rust/description
  Rust is a multi-paradigm, general-purpose programming language designed for performance
  and safety, especially safe concurrency. Rust is syntactically similar to C++, but can
  guarantee memory safety by using a borrow checker to validate references.
endef

define Package/rust/config
	source "$(SOURCE)/Config.in"
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,rust))
