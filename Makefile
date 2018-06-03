include $(TOPDIR)/rules.mk

PKG_NAME:=firstatt
PKG_VERSION:=1.2
PKG_RELEASE:=1
#OpenWrt support many ways to download the source code, specify it
PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/AlexandraDrobut/OpenWrtPack.git
PKG_SOURCE_DATE:=2018-05-30
PKG_SOURCE_VERSION:=07fb9c9e90b6eba7a7b10b22110f34d3273c6c2b
#The hash is generated in order to check the tarball : do make package/../name/check FIXUP=1 V=s
PKG_MIRROR_HASH:=ede7bda24fa4d42d90ea24d88afc3fa924892ce49bc8364d1dcf6a1f49a5c0aa

include $(INCLUDE_DIR)/package.mk

#Section where the package will be instaled in the configuration structure
define Package/$(PKG_NAME)
  SECTION:=base
  CATEGORY:=Base system
  TITLE:=Netlink utility
  URL:=https://github.com/AlexandraDrobut/OpenWrtPackys
endef

define Package/$(PKG_NAME)/description
	Trying to create a first package, just for learning purpose
	Sending a netlink request to the kernel routing table in order to find the interface names and their default gateway.
endef
#Where is the package installed in the firmware image structure/ 
define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/hello $(1)/usr/bin/firstatt
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
