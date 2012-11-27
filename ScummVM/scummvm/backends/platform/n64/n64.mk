N64_EXE_STRIPPED := scummvm_stripped$(EXEEXT)

bundle_name = n64-dist/scummvm
BASESIZE = 2097152

all: $(N64_EXE_STRIPPED)

$(N64_EXE_STRIPPED): $(EXECUTABLE)
	$(STRIP) $< -o $@

n64-distclean:
	rm -rf $(bundle_name)
	rm $(N64_EXE_STRIPPED)

n64-dist: all
	$(MKDIR) $(bundle_name)
	$(MKDIR) $(bundle_name)/romfs
ifdef DIST_FILES_ENGINEDATA
	$(CP) $(DIST_FILES_ENGINEDATA) $(bundle_name)/romfs
endif
	$(CP) $(DIST_FILES_DOCS) $(bundle_name)/
	$(CP) $(srcdir)/backends/vkeybd/packs/vkeybd_default.zip $(bundle_name)/romfs
	genromfs -f $(bundle_name)/romfs.img -d $(bundle_name)/romfs -V scummvmn64
	mips64-objcopy $(EXECUTABLE) $(bundle_name)/scummvm.elf -O binary
	cat $(N64SDK)/hkz-libn64/bootcode $(bundle_name)/scummvm.elf $(bundle_name)/romfs.img > scummvm.v64
	$(srcdir)/backends/platform/n64/pad_rom.sh scummvm.v64
	rm scummvm.bak
	mv scummvm.v64 $(bundle_name)/scummvm.v64
