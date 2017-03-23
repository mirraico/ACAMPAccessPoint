include $(TOPDIR)/rules.mk

PKG_NAME:=acamp
PKG_RELEASE:=0.3

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/acamp
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=acamp
	DEPENDS:=+libuci
endef

define Package/acamp/description
	acamp agent for ap.
endef

define Build/Prepare
	echo "ACAMP Package/Prepare"
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/acamp/install
	echo "ACAMP Package/install"
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/acamp $(1)/bin/
endef

$(eval $(call BuildPackage,acamp))

