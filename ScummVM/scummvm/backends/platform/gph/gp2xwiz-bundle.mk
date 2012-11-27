# Special target to create bundles for the GP2X Wiz.

bundle_name = release/scummvm-gp2xwiz
f=$(shell which $(STRIP))
libloc = $(shell dirname $(f))

gp2xwiz-bundle: $(EXECUTABLE)
	$(MKDIR) "$(bundle_name)"
	$(MKDIR) "$(bundle_name)/scummvm"
	$(MKDIR) "$(bundle_name)/scummvm/saves"
	$(MKDIR) "$(bundle_name)/scummvm/engine-data"
	$(MKDIR) "$(bundle_name)/scummvm/lib"

	echo "Please put your save games in this dir" >> "$(bundle_name)/scummvm/saves/PUT_SAVES_IN_THIS_DIR"

	$(CP) $(srcdir)/dists/gph/gp2xwiz/scummvm.gpe $(bundle_name)/scummvm/
	$(CP) $(srcdir)/dists/gph/scummvm.png $(bundle_name)/scummvm/
	$(CP) $(srcdir)/dists/gph/scummvmb.png $(bundle_name)/scummvm/
	$(CP) $(srcdir)/dists/gph/README-GPH $(bundle_name)/scummvm/
	$(CP) $(srcdir)/dists/gph/scummvm.ini $(bundle_name)/

	$(INSTALL) -c -m 644 $(DIST_FILES_DOCS) $(bundle_name)/scummvm/
	$(INSTALL) -c -m 644 $(DIST_FILES_THEMES) $(bundle_name)/scummvm/
	$(INSTALL) -c -m 644 $(DIST_FILES_ENGINEDATA) $(bundle_name)/scummvm/engine-data
	$(CP) $(srcdir)/backends/vkeybd/packs/vkeybd_default.zip $(bundle_name)/scummvm/

	$(STRIP) $(EXECUTABLE) -o $(bundle_name)/scummvm/$(EXECUTABLE)

ifdef DYNAMIC_MODULES
	$(INSTALL) -d "$(bundle_name)/scummvm/plugins"
	$(INSTALL) -c -m 644 $(PLUGINS) "$(bundle_name)/scummvm/plugins"
	$(STRIP) $(bundle_name)/scummvm/plugins/*
endif

	$(CP) $(libloc)/../lib/libz.so.1.2.3 $(bundle_name)/scummvm/lib/libz.so.1
	$(CP) $(libloc)/../lib/libvorbisidec.so.1.0.2 $(bundle_name)/scummvm/lib/libvorbisidec.so.1

	tar -C $(bundle_name) -cvjf $(bundle_name).tar.bz2 .
	rm -R ./$(bundle_name)

gp2xwiz-bundle-debug: $(EXECUTABLE)
	$(MKDIR) "$(bundle_name)"
	$(MKDIR) "$(bundle_name)/scummvm"
	$(MKDIR) "$(bundle_name)/scummvm/saves"
	$(MKDIR) "$(bundle_name)/scummvm/engine-data"
	$(MKDIR) "$(bundle_name)/scummvm/lib"

	echo "Please put your save games in this dir" >> "$(bundle_name)/scummvm/saves/PUT_SAVES_IN_THIS_DIR"

	$(CP) $(srcdir)/dists/gph/gp2xwiz/scummvm-gdb.gpe $(bundle_name)/scummvm/scummvm.gpe
	$(CP) $(srcdir)/dists/gph/scummvm.png $(bundle_name)/scummvm/
	$(CP) $(srcdir)/dists/gph/scummvmb.png $(bundle_name)/scummvm/
	$(CP) $(srcdir)/dists/gph/README-GPH $(bundle_name)/scummvm/
	$(CP) $(srcdir)/dists/gph/scummvm.ini $(bundle_name)/

	$(INSTALL) -c -m 644 $(DIST_FILES_DOCS) $(bundle_name)/scummvm/
	$(INSTALL) -c -m 644 $(DIST_FILES_THEMES) $(bundle_name)/scummvm/
	$(INSTALL) -c -m 644 $(DIST_FILES_ENGINEDATA) $(bundle_name)/scummvm/engine-data
	$(CP) $(srcdir)/backends/vkeybd/packs/vkeybd_default.zip $(bundle_name)/scummvm/

	$(INSTALL) -c -m 777 $(srcdir)/$(EXECUTABLE) $(bundle_name)/scummvm/$(EXECUTABLE)

ifdef DYNAMIC_MODULES
	$(INSTALL) -d "$(bundle_name)/scummvm/plugins"
	$(INSTALL) -c -m 644 $(PLUGINS) "$(bundle_name)/scummvm/plugins"
endif

	$(CP) $(libloc)/../lib/libz.so.1.2.3 $(bundle_name)/scummvm/lib/libz.so.1
	$(CP) $(libloc)/../lib/libvorbisidec.so.1.0.2 $(bundle_name)/scummvm/lib/libvorbisidec.so.1

	tar -C $(bundle_name) -cvjf $(bundle_name)-debug.tar.bz2 .
	rm -R ./$(bundle_name)

.PHONY: gp2xwiz-bundle gp2xwiz-bundle-debug
