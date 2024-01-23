include $(TOPDIR)/rules.mk

PKG_NAME:=wazuh-agent
PKG_VERSION:=4.7.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/wazuh-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL=https://codeload.github.com/wazuh/wazuh/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=1c505f66fc28a702f58867e63117ab470eb2ba4dafc84c2fc30a27a2b29cb980

PKG_MAINTAINER:=HaiBac <mail@bacnh.com>
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/wazuh-agent
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Wazuh Agent
  URL:=https://github.com/wazuh/wazuh
  DEPENDS:=+perl +perlbase-findbin +perlbase-pod +perlbase-storable +perlbase-feature +libpthread +librt +libintl-full
endef

define Package/wazuh-agent/description
  Wazuh is an open source unified XDR (Extended Detection and Response) and SIEM (Security Information en Event Management)
  system capable of offering protection for endpoints and cloud workloads.
endef

MAKE_FLAGS+= \
	PLATFORM=Linux \
	OPTFLAGS="$(TARGET_CXXFLAGS)" \
	LDCXXFLAGS="$(TARGET_LDFLAGS) -pthread"

define Build/Compile
  cd $(PKG_BUILD_DIR)/src && \
  export TARGET_CONFIGURE_OPTS='$(TARGET_CONFIGURE_OPTS)' && \
  export CONFIGURE_ARGS='$(CONFIGURE_ARGS)' && \
  export TARGET_LDFLAGS='$(TARGET_LDFLAGS)' && \
  $(MAKE) $(TARGET_CONcFIGURE_OPTS) EXTERNAL_SRC_ONLY=yes deps && \
  $(MAKE) $(TARGET_CONFIGURE_OPTS) TARGET=agent
endef

define Package/wazuh-agent/install
	$(INSTALL_DIR) $(1)/usr/bin
endef

$(eval $(call BuildPackage,wazuh-agent))