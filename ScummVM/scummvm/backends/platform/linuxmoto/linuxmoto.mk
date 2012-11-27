# Special target to create a motoezx snapshot
motoezx: $(EXECUTABLE)
	$(MKDIR) release/scummvm
	$(STRIP) $(EXECUTABLE) -o release/scummvm/$(EXECUTABLE)
	$(INSTALL) -c -m 644 $(DIST_FILES_THEMES) $(DIST_FILES_ENGINEDATA) release/scummvm/
	$(INSTALL) -c -m 644 $(DIST_FILES_DOCS)  release/scummvm/
	$(CP) $(srcdir)/backends/vkeybd/packs/vkeybd_default.zip release/scummvm/
	$(CP) $(srcdir)/dists/motoezx/* release/scummvm/
	tar -C release -cvzf release/ScummVM-motoezx.pkg scummvm
	$(RM) -r release/scummvm

# Special target to create a motomagx snapshot
motomagx-mpkg: $(EXECUTABLE)
	$(MKDIR) release/scummvm
	$(STRIP) $(EXECUTABLE) -o release/scummvm/$(EXECUTABLE)
	$(INSTALL) -c -m 644 $(DIST_FILES_THEMES) $(DIST_FILES_ENGINEDATA) release/scummvm/
	$(INSTALL) -c -m 644 $(DIST_FILES_DOCS)  release/scummvm/
	$(CP) $(srcdir)/backends/vkeybd/packs/vkeybd_default.zip release/scummvm/
	$(CP) $(srcdir)/dists/motomagx/mpkg/* release/scummvm/
	tar -C release -cvzf release/ScummVM-motomagx.mpkg scummvm
	$(RM) -r release/scummvm

motomagx-mgx: $(EXECUTABLE)
	$(MKDIR) release/scummvm
	$(STRIP) $(EXECUTABLE) -o release/scummvm/$(EXECUTABLE)
	$(INSTALL) -c -m 644 $(DIST_FILES_THEMES) $(DIST_FILES_ENGINEDATA) release/scummvm/
	$(INSTALL) -c -m 644 $(DIST_FILES_DOCS)  release/scummvm/
	$(CP) $(srcdir)/backends/vkeybd/packs/vkeybd_default.zip release/scummvm/
	$(CP) $(srcdir)/dists/motomagx/mgx/* release/scummvm/
	tar -C release -cvf release/ScummVM-motomagx.mgx scummvm
	$(RM) -r release/scummvm

motomagx-pep: $(EXECUTABLE)
	$(MKDIR) release/pep
	$(CP) -r $(srcdir)/dists/motomagx/pep/* release/pep
	$(RM) -r release/pep/app/.svn
	$(STRIP) $(EXECUTABLE) -o release/pep/app/$(EXECUTABLE)
	$(INSTALL) -c -m 644 $(DIST_FILES_THEMES) $(DIST_FILES_ENGINEDATA) release/pep/app
	$(INSTALL) -c -m 644 $(DIST_FILES_DOCS)  release/pep/app
	$(CP) $(srcdir)/backends/vkeybd/packs/vkeybd_default.zip release/pep/app
	tar -C release/pep -czvf release/ScummVM-motomagx.pep app description.ini  scummvm_big_usr.png  scummvm_small_usr.png
	$(RM) -r release/pep

.PHONY: motoezx motomagx-mpkg motomagx-mgx motomagx-pep
