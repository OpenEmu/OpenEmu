# Special target to create bundles and PND's for the OpenPandora.

#bundle_name = release/scummvm-op-`date '+%Y-%m-%d'`
bundle_name = release/scummvm-op
f=$(shell which $(STRIP))
libloc = $(shell dirname $(f))

op-bundle: $(EXECUTABLE)
	$(MKDIR) "$(bundle_name)"
	$(MKDIR) "$(bundle_name)/scummvm"
	$(MKDIR) "$(bundle_name)/scummvm/bin"
	$(MKDIR) "$(bundle_name)/scummvm/data"
	$(MKDIR) "$(bundle_name)/scummvm/docs"
	$(MKDIR) "$(bundle_name)/scummvm/icon"
	$(MKDIR) "$(bundle_name)/scummvm/lib"

	$(CP) $(srcdir)/dists/openpandora/runscummvm.sh $(bundle_name)/scummvm/
	$(CP) $(srcdir)/dists/openpandora/PXML.xml $(bundle_name)/scummvm/data/

	$(CP) $(srcdir)/dists/openpandora/icon/scummvm.png $(bundle_name)/scummvm/icon/
	$(CP) $(srcdir)/dists/openpandora/icon/preview-pic.png  $(bundle_name)/scummvm/icon/


	$(CP) $(srcdir)/dists/openpandora/README-OPENPANDORA $(bundle_name)/scummvm/docs/
	$(CP) $(srcdir)/dists/openpandora/index.html $(bundle_name)/scummvm/docs/

	$(INSTALL) -c -m 644 $(DIST_FILES_DOCS) $(bundle_name)/scummvm/docs/

	$(INSTALL) -c -m 644 $(DIST_FILES_THEMES) $(bundle_name)/scummvm/data/
	$(INSTALL) -c -m 644 $(DIST_FILES_ENGINEDATA) $(bundle_name)/scummvm/data/

	$(STRIP) $(EXECUTABLE) -o $(bundle_name)/scummvm/bin/$(EXECUTABLE)

ifdef DYNAMIC_MODULES
	$(INSTALL) -d "$(bundle_name)/scummvm/plugins"
	$(INSTALL) -c -m 644 $(PLUGINS) "$(bundle_name)/scummvm/plugins"
	$(STRIP) $(bundle_name)/scummvm/plugins/*
endif

	$(CP) $(libloc)/../arm-angstrom-linux-gnueabi/usr/lib/libFLAC.so.8.2.0 $(bundle_name)/scummvm/lib/libFLAC.so.8
	tar -C $(bundle_name) -cvjf $(bundle_name).tar.bz2 .
	rm -R ./$(bundle_name)

op-pnd: $(EXECUTABLE)
	$(MKDIR) "$(bundle_name)"
	$(MKDIR) "$(bundle_name)/scummvm"
	$(MKDIR) "$(bundle_name)/scummvm/bin"
	$(MKDIR) "$(bundle_name)/scummvm/data"
	$(MKDIR) "$(bundle_name)/scummvm/docs"
	$(MKDIR) "$(bundle_name)/scummvm/icon"
	$(MKDIR) "$(bundle_name)/scummvm/lib"

	$(CP) $(srcdir)/dists/openpandora/runscummvm.sh $(bundle_name)/scummvm/
	$(CP) $(srcdir)/dists/openpandora/PXML.xml $(bundle_name)/scummvm/data/

	$(CP) $(srcdir)/dists/openpandora/icon/scummvm.png $(bundle_name)/scummvm/icon/
	$(CP) $(srcdir)/dists/openpandora/icon/preview-pic.png  $(bundle_name)/scummvm/icon/


	$(CP) $(srcdir)/dists/openpandora/README-OPENPANDORA $(bundle_name)/scummvm/docs/
	$(CP) $(srcdir)/dists/openpandora/index.html $(bundle_name)/scummvm/docs/

	$(INSTALL) -c -m 644 $(DIST_FILES_DOCS) $(bundle_name)/scummvm/docs/

	$(INSTALL) -c -m 644 $(DIST_FILES_THEMES) $(bundle_name)/scummvm/data/
	$(INSTALL) -c -m 644 $(DIST_FILES_ENGINEDATA) $(bundle_name)/scummvm/data/

	$(STRIP) $(EXECUTABLE) -o $(bundle_name)/scummvm/bin/$(EXECUTABLE)

ifdef DYNAMIC_MODULES
	$(INSTALL) -d "$(bundle_name)/scummvm/plugins"
	$(INSTALL) -c -m 644 $(PLUGINS) "$(bundle_name)/scummvm/plugins"
	$(STRIP) $(bundle_name)/scummvm/plugins/*
endif

	$(CP) $(libloc)/../arm-angstrom-linux-gnueabi/usr/lib/libFLAC.so.8.2.0 $(bundle_name)/scummvm/lib/libFLAC.so.8

	$(srcdir)/dists/openpandora/pnd_make.sh -p $(bundle_name).pnd -c -d $(bundle_name)/scummvm -x $(bundle_name)/scummvm/data/PXML.xml -i $(bundle_name)/scummvm/icon/scummvm.png

	$(CP) $(srcdir)/dists/openpandora/README-PND.txt $(bundle_name)

	tar -cvjf $(bundle_name)-pnd.tar.bz2 $(bundle_name).pnd $(bundle_name)/README-PND.txt
	rm -R ./$(bundle_name)

.PHONY: op-bundle op-pnd
