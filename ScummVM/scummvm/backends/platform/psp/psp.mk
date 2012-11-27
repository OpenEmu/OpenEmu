PSP_EXE_STRIPPED := scummvm_stripped$(EXEEXT)
PSP_EBOOT = EBOOT.PBP
PSP_EBOOT_SFO = param.sfo
PSP_EBOOT_TITLE = ScummVM-PSP
DATE = $(shell date +%Y%m%d)

MKSFO = mksfoex -d MEMSIZE=1
PACK_PBP = pack-pbp

all: pack_pbp

clean: psp_clean

$(PSP_EXE_STRIPPED): $(EXECUTABLE)
	$(STRIP) $< -o $@

$(PSP_EBOOT_SFO): $(EXECUTABLE)
	$(MKSFO) '$(PSP_EBOOT_TITLE) r$(VER_REV) ($(DATE))' $@

psp_clean:
	$(RM) $(PSP_EXE_STRIPPED) $(PSP_EBOOT) $(PSP_EBOOT_SFO)

psp_fixup_elf: $(PSP_EXE_STRIPPED)
	psp-fixup-imports $<

pack_pbp: psp_fixup_elf $(PSP_EBOOT_SFO)
	$(PACK_PBP) $(PSP_EBOOT) \
	$(PSP_EBOOT_SFO) \
	$(srcdir)/backends/platform/psp/icon0.png \
	NULL \
	$(srcdir)/backends/platform/psp/pic0.png \
	$(srcdir)/backends/platform/psp/pic1.png \
	NULL \
	$(PSP_EXE_STRIPPED) \
	NULL

.PHONY: psp_fixup_elf pack_pbp
