objects := ui-main ui-base ui-cartridge ui-debugger ui-input ui-movie ui-settings ui-state ui-tools $(objects)
objects += $(if $(call streq,$(platform),win),resource)
link += $(qtlib)

headers := $(call rwildcard,$(ui)/,%.hpp)
moc_headers := $(call rwildcard,$(ui)/,%.moc.hpp)
moc_objects := $(foreach f,$(moc_headers),obj/$(notdir $(patsubst %.moc.hpp,%.moc,$f)))
qt_compile = $(call compile,-Iobj $(qtinc))

#############
### rules ###
#############

# automatically run moc on all .moc.hpp (MOC header) files
%.moc: $<; $(moc) -i $< -o $@

# automatically generate %.moc build rules
__list = $(moc_headers)
$(foreach f,$(moc_objects), \
  $(eval __file = $(word 1,$(__list))) \
  $(eval __list = $(wordlist 2,$(words $(__list)),$(__list))) \
  $(eval $f: $(__file)) \
)

obj/ui-main.o: $(ui)/main.cpp $(headers) $(wildcard $(ui)/*.cpp) $(wildcard $(ui)/platform/*.cpp) $(wildcard $(ui)/utility/*.cpp); $(qt_compile)
obj/ui-base.o: $(ui)/base/base.cpp $(headers) $(wildcard $(ui)/base/*.cpp); $(qt_compile)
obj/ui-cartridge.o: $(ui)/cartridge/cartridge.cpp $(headers) $(wildcard $(ui)/cartridge/*.cpp); $(qt_compile)
obj/ui-debugger.o: $(ui)/debugger/debugger.cpp $(headers) $(call rwildcard,$(ui)/debugger/,%.cpp); $(qt_compile)
obj/ui-input.o: $(ui)/input/input.cpp $(headers) $(wildcard $(ui)/input/*.cpp); $(qt_compile)
obj/ui-movie.o: $(ui)/movie/movie.cpp $(headers) $(wildcard $(ui)/movie/*.cpp); $(qt_compile)
obj/ui-settings.o: $(ui)/settings/settings.cpp $(headers) $(wildcard $(ui)/settings/*.cpp); $(qt_compile)
obj/ui-state.o: $(ui)/state/state.cpp $(headers) $(wildcard $(ui)/state/*.cpp); $(qt_compile)
obj/ui-tools.o: $(ui)/tools/tools.cpp $(headers) $(wildcard $(ui)/tools/*.cpp); $(qt_compile)

obj/resource.rcc: $(ui)/resource/resource.qrc data/*
	$(rcc) $(ui)/resource/resource.qrc -o obj/resource.rcc

obj/resource.o: $(ui)/resource/resource.rc
	windres $(ui)/resource/resource.rc obj/resource.o

###############
### targets ###
###############

ui_build: obj/resource.rcc $(moc_objects);
ui_clean:
	-@$(call delete,obj/*.rcc)
	-@$(call delete,obj/*.moc)
