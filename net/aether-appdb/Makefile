#
# Copyright (C) 2026 Optim Enterprises BV
#
# THE DATABASE IS DATA, AND IT IS NOT PUBLISHED.
#
# files/appdb.cfg is the 1,347-application signature database aether-sensord
# classifies against. It is carried here, in this private feed, rather than in
# the aether-sensord repository, which is public under BUSL-1.1.
#
# PROVENANCE IS NOT ESTABLISHED (2026-08-22). Traced: it is NOT destan19's data
# -- every OpenAppFilter release ships 87 apps / 5 classes in 1.5-3.3 KB, this
# is 1,347 apps / 32 classes in 56 KB, and the two share no id, no application
# name and no identical entry. It entered via optim-enterprises-bv/OpenAppFilter
# (private) in a single commit, 3fa19dd2 2023-08-16, +1414 -97, stamped
# "#version v11.08.23". A commit of that shape is an import, not authorship, and
# where it was imported from is not recorded anywhere we can see.
#
# Publishing it would assert ownership we cannot currently evidence, so it stays
# here until that is settled. Moving it into the public repository is a one-line
# change once it is.
#
# It is also not very good: 64% of its applications match only 'www.<domain>' --
# an exact hostname, usually the marketing site rather than the endpoint the
# application uses. Blocking WhatsApp with it stops the brochure page and
# nothing else, while every counter reports success. See SIGNATURE-COVERAGE.md
# in the aether-sensord repository. Replacing it with data we author fixes the
# licensing question and the coverage gap in the same stroke.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=aether-appdb
PKG_VERSION:=11.08.23
PKG_RELEASE:=1

PKG_MAINTAINER:=Optim Enterprises BV
# Deliberately not a licence identifier. See the provenance note above: we do
# not know what licence applies, and claiming one would be worse than saying so.
PKG_LICENSE:=proprietary

include $(INCLUDE_DIR)/package.mk

define Package/aether-appdb
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Firewall
  TITLE:=Application signature database for aether-sensord
  URL:=https://github.com/optim-enterprises-bv/optim-wrt
  DEPENDS:=
endef

define Package/aether-appdb/description
  The application signature database aether-sensord classifies flows against:
  1,347 applications across 32 classes.

  Split out of the retired open-app-filter package so that aether-sensord has a
  declared dependency on the data it needs. Previously the daemon read a path
  that only another package happened to install, so installing aether-sensord
  alone produced a daemon that loaded zero signatures, logged as much, and
  blocked nothing.

  KNOWN COVERAGE LIMIT: 64% of applications match only 'www.<domain>', an exact
  hostname. For most of those the www host is the marketing site rather than the
  endpoint the application talks to, so blocking them has no effect on the
  application. Measured; see SIGNATURE-COVERAGE.md in the aether-sensord
  repository before trusting an app block.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/aether-appdb/install
	$(INSTALL_DIR) $(1)/usr/share/aether
	$(INSTALL_DATA) ./files/appdb.cfg $(1)/usr/share/aether/appdb.cfg

	# Compatibility symlink at the path open-app-filter used to install.
	#
	# UCI config files survive sysupgrade, so a device upgraded from an older
	# build still has db_path='/etc/appfilter/feature_en.cfg' in
	# /etc/config/aether-sensord. Without this link that path stops existing
	# and the daemon loads zero signatures -- silently, on exactly the devices
	# that were working before the upgrade.
	$(INSTALL_DIR) $(1)/etc/appfilter
	$(LN) /usr/share/aether/appdb.cfg $(1)/etc/appfilter/feature_en.cfg
endef

$(eval $(call BuildPackage,aether-appdb))
