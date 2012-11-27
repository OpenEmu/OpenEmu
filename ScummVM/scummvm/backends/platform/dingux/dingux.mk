DINGUX_EXE_STRIPPED := scummvm_stripped$(EXEEXT)

bundle_name = dingux-dist/scummvm

all: $(DINGUX_EXE_STRIPPED)

$(DINGUX_EXE_STRIPPED): $(EXECUTABLE)
	$(STRIP) $< -o $@

dingux-distclean:
	rm -rf $(bundle_name)
	rm $(DINGUX_EXE_STRIPPED)

dingux-dist: all
	$(MKDIR) $(bundle_name)
	$(MKDIR) $(bundle_name)/saves
	$(STRIP) $(EXECUTABLE) -o $(bundle_name)/scummvm.elf
	$(CP) $(DIST_FILES_THEMES) $(bundle_name)/
ifdef DIST_FILES_ENGINEDATA
	$(CP) $(DIST_FILES_ENGINEDATA) $(bundle_name)/
endif
	$(CP) $(DIST_FILES_DOCS) $(bundle_name)/
ifdef DYNAMIC_MODULES
		$(MKDIR) $(bundle_name)/plugins
		$(CP) $(PLUGINS) $(bundle_name)/plugins
		$(STRIP) $(bundle_name)/plugins/*
endif
	$(CP) $(srcdir)/backends/vkeybd/packs/vkeybd_default.zip $(bundle_name)/
	$(CP) $(srcdir)/backends/platform/dingux/scummvm.gpe $(bundle_name)/
	$(CP) $(srcdir)/backends/platform/dingux/README.DINGUX $(bundle_name)/
	$(CP) $(srcdir)/backends/platform/dingux/scummvm.png $(bundle_name)/
