objects := ui-main $(objects)
objects += $(if $(call streq,$(platform),win),resource)
link += $(qtlib)

moc_headers := $(call rwildcard,$(ui)/,%.moc.hpp)
moc_objects := $(patsubst %.moc.hpp,%.moc,$(moc_headers))

#############
### rules ###
#############

# automatically run moc on all .moc.hpp (MOC header) files
%.moc: $<; $(moc) -f $< -o $@
$(foreach f,$(moc_objects),$(eval $f: $(patsubst %.moc,%.moc.hpp,$f)))

obj/ui-main.o: $(ui)/main.cpp $(call rwildcard,$(ui)/)
	$(call compile,$(qtinc))

$(ui)/resource/resource.rcc: $(ui)/resource/resource.qrc data/*
	$(rcc) $(ui)/resource/resource.qrc -o $(ui)/resource/resource.rcc

obj/resource.o: $(ui)/resource/resource.rc
	windres $(ui)/resource/resource.rc obj/resource.o

###############
### targets ###
###############

ui_build: $(ui)/resource/resource.rcc $(moc_objects);
ui_clean:
	-$(foreach f,$(moc_objects),@$(call delete,$f))
	-@$(call delete,$(ui)/resource/resource.rcc)
