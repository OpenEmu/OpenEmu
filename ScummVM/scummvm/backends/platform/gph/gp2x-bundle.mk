# Special target to create bundles for the GP2X.

bundle_name = release/scummvm-gp2x
f=$(shell which $(STRIP))
libloc = $(shell dirname $(f))

gp2x-bundle: $(EXECUTABLE)
	$(MKDIR) "$(bundle_name)"
	$(MKDIR) "$(bundle_name)/saves"
	$(MKDIR) "$(bundle_name)/engine-data"
	$(MKDIR) "$(bundle_name)/lib"

	echo "Please put your save games in this dir" >> "$(bundle_name)/saves/PUT_SAVES_IN_THIS_DIR"

	$(CP) $(srcdir)/dists/gph/gp2x/scummvm.gpe $(bundle_name)
	$(CP) $(srcdir)/dists/gph/scummvm.png $(bundle_name)
	$(CP) $(srcdir)/dists/gph/README-GPH $(bundle_name)
	$(CP) $(srcdir)/dists/gph/gp2x/mmuhack/mmuhack.o $(bundle_name)

	$(INSTALL) -c -m 644 $(DIST_FILES_DOCS) $(bundle_name)
	$(INSTALL) -c -m 644 $(DIST_FILES_THEMES) $(bundle_name)
	$(INSTALL) -c -m 644 $(DIST_FILES_ENGINEDATA) $(bundle_name)/engine-data
	$(CP) $(srcdir)/backends/vkeybd/packs/vkeybd_default.zip $(bundle_name)

	$(STRIP) $(EXECUTABLE) -o $(bundle_name)/$(EXECUTABLE)

ifdef DYNAMIC_MODULES
	$(INSTALL) -d "$(bundle_name)/plugins"
	$(INSTALL) -c -m 644 $(PLUGINS) "$(bundle_name)/plugins"
	$(STRIP) $(bundle_name)/plugins/*
endif

	$(CP) $(libloc)/../arm-open2x-linux/lib/libdl.so $(bundle_name)/lib/libdl.so

	tar -C $(bundle_name) -cvjf $(bundle_name).tar.bz2 .
	rm -R ./$(bundle_name)

gp2x-bundle-debug: $(EXECUTABLE)
	$(MKDIR) "$(bundle_name)"
	$(MKDIR) "$(bundle_name)/saves"
	$(MKDIR) "$(bundle_name)/engine-data"
	$(MKDIR) "$(bundle_name)/lib"

	echo "Please put your save games in this dir" >> "$(bundle_name)/saves/PUT_SAVES_IN_THIS_DIR"

	$(CP) $(srcdir)/dists/gph/gp2x/scummvm.gpe $(bundle_name)
	$(CP) $(srcdir)/dists/gph/scummvm.png $(bundle_name)
	$(CP) $(srcdir)/dists/gph/README-GPH $(bundle_name)
	$(CP) $(srcdir)/dists/gph/gp2x/mmuhack/mmuhack.o $(bundle_name)

	$(INSTALL) -c -m 644 $(DIST_FILES_DOCS) $(bundle_name)
	$(INSTALL) -c -m 644 $(DIST_FILES_THEMES) $(bundle_name)
	$(INSTALL) -c -m 644 $(DIST_FILES_ENGINEDATA) $(bundle_name)/engine-data
	$(CP) $(srcdir)/backends/vkeybd/packs/vkeybd_default.zip $(bundle_name)

	$(INSTALL) -c -m 777 $(srcdir)/$(EXECUTABLE) $(bundle_name)/$(EXECUTABLE)

ifdef DYNAMIC_MODULES
	$(INSTALL) -d "$(bundle_name)/scummvm/plugins"
	$(INSTALL) -c -m 644 $(PLUGINS) "$(bundle_name)/scummvm/plugins"
endif

	$(CP) $(libloc)/../arm-open2x-linux/lib/libdl.so $(bundle_name)/lib/libdl.so

	tar -C $(bundle_name) -cvjf $(bundle_name)-debug.tar.bz2 .
	rm -R ./$(bundle_name)

.PHONY: gp2x-bundle gp2x-bundle-debug
