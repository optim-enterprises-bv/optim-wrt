# Rust Environmental Vars
CONFIG_HOST_SUFFIX:=$(shell cut -d"-" -f4 <<<"$(GNU_HOST_NAME)")
RUSTC_HOST_ARCH:=$(HOST_ARCH)-unknown-linux-$(CONFIG_HOST_SUFFIX)
RUSTC_TARGET_ARCH:=$(REAL_GNU_TARGET_NAME)
CARGO_HOME:=$(STAGING_DIR_HOST)

# These RUSTFLAGS are common across all TARGETs
RUSTFLAGS = "-C linker=$(TOOLCHAIN_DIR)/bin/$(TARGET_CC_NOCACHE) -C ar=$(TOOLCHAIN_DIR)/bin/$(TARGET_AR)"

# Common Build Flags
RUST_BUILD_FLAGS = \
  RUSTFLAGS=$(RUSTFLAGS) \
  CARGO_HOME="$(CARGO_HOME)"

# This adds the rust environmental variables to Make calls
MAKE_FLAGS += $(RUST_BUILD_FLAGS)

# ARM Logic
ifeq ($(ARCH),"arm")
  ifeq ($(CONFIG_arm_v7),y)
    RUSTC_TARGET_ARCH:=$(subst arm,armv7,$(RUSTC_TARGET_ARCH))
  endif

  ifeq ($(CONFIG_HAS_FPU),y)
    RUSTC_TARGET_ARCH:=$(subst muslgnueabi,muslgnueabihf,$(RUSTC_TARGET_ARCH))
  endif
endif

# These allow Cargo packaged projects to be compile via $(call xxx/Compile/Cargo)
define Host/Compile/Cargo
	cd $(PKG_BUILD_DIR) && CARGO_HOME=$(CARGO_HOME) RUSTFLAGS=$(RUSTFLAGS) cargo update && \
	  CARGO_HOME=$(CARGO_HOME) RUSTFLAGS=$(RUSTFLAGS) cargo build -v --release \
	  --target $(RUSTC_TARGET_ARCH),$(RUST_HOST_ARCH)
endef

define Build/Compile/Cargo
	cd $(PKG_BUILD_DIR) && CARGO_HOME=$(CARGO_HOME) RUSTFLAGS=$(RUSTFLAGS) cargo update && \
	  CARGO_HOME=$(CARGO_HOME) RUSTFLAGS=$(RUSTFLAGS) cargo build -v --release --target $(RUSTC_TARGET_ARCH)
endef
