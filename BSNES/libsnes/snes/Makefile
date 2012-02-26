snes_objects := libco
snes_objects += snes-interface snes-system snes-controller
snes_objects += snes-cartridge snes-cheat
snes_objects += snes-memory snes-cpucore snes-smpcore
snes_objects += snes-cpu snes-smp snes-dsp snes-ppu
snes_objects += snes-nss snes-icd2 snes-superfx snes-sa1 snes-necdsp snes-hitachidsp
snes_objects += snes-bsx snes-srtc snes-sdd1 snes-spc7110 snes-sufamiturbo
snes_objects += snes-obc1 snes-st0018
snes_objects += snes-msu1 snes-link
snes_objects += $(gameboy_objects)
objects += $(snes_objects)

ifeq ($(profile),accuracy)
  extraflags += -DPROFILE_ACCURACY
  snescpu := $(snes)/cpu
  snessmp := $(snes)/smp
  snesdsp := $(snes)/dsp
  snesppu := $(snes)/ppu
else ifeq ($(profile),compatibility)
  extraflags += -DPROFILE_COMPATIBILITY
  snescpu := $(snes)/cpu
  snessmp := $(snes)/smp
  snesdsp := $(snes)/alt/dsp
  snesppu := $(snes)/alt/ppu-compatibility
else ifeq ($(profile),performance)
  extraflags += -DPROFILE_PERFORMANCE
  snescpu := $(snes)/alt/cpu
  snessmp := $(snes)/alt/smp
  snesdsp := $(snes)/alt/dsp
  snesppu := $(snes)/alt/ppu-performance
endif

obj/libsnes.o: $(snes)/libsnes/libsnes.cpp $(snes)/libsnes/* 

obj/libco.o         : libco/libco.c libco/*

obj/snes-interface.o : $(snes)/interface/interface.cpp $(call rwildcard,$(snes)/interface)
obj/snes-system.o    : $(snes)/system/system.cpp $(call rwildcard,$(snes)/system/)
obj/snes-controller.o: $(snes)/controller/controller.cpp $(call rwildcard,$(snes)/controller/)
obj/snes-memory.o    : $(snes)/memory/memory.cpp $(call rwildcard,$(snes)/memory/)
obj/snes-cpucore.o   : $(snes)/cpu/core/core.cpp $(call rwildcard,$(snes)/cpu/core/)
obj/snes-smpcore.o   : $(snes)/smp/core/smp_core.cpp $(call rwildcard,$(snes)/smp/core/)
obj/snes-cpu.o       : $(snescpu)/cpu.cpp $(call rwildcard,$(snescpu)/)
obj/snes-smp.o       : $(snessmp)/smp.cpp $(call rwildcard,$(snessmp)/)
obj/snes-dsp.o       : $(snesdsp)/dsp.cpp $(call rwildcard,$(snesdsp)/)
obj/snes-ppu.o       : $(snesppu)/ppu.cpp $(call rwildcard,$(snesppu)/)
obj/snes-cartridge.o : $(snes)/cartridge/cartridge.cpp $(snes)/cartridge/*
obj/snes-cheat.o     : $(snes)/cheat/cheat.cpp $(snes)/cheat/*

obj/snes-nss.o     : $(snes)/chip/nss/nss.cpp $(call rwildcard,$(snes)/chip/nss/)
obj/snes-icd2.o    : $(snes)/chip/icd2/icd2.cpp $(call rwildcard,$(snes)/chip/icd2/)
obj/snes-superfx.o : $(snes)/chip/superfx/superfx.cpp $(call rwildcard,$(snes)/chip/superfx/)
obj/snes-sa1.o     : $(snes)/chip/sa1/sa1.cpp $(call rwildcard,$(snes)/chip/sa1/)
obj/snes-necdsp.o  : $(snes)/chip/necdsp/necdsp.cpp $(call rwildcard,$(snes)/chip/necdsp/)
obj/snes-hitachidsp.o : $(snes)/chip/hitachidsp/hitachidsp.cpp $(call rwildcard,$(snes)/chip/hitachidsp/)
obj/snes-bsx.o     : $(snes)/chip/bsx/bsx.cpp $(snes)/chip/bsx/*
obj/snes-srtc.o    : $(snes)/chip/srtc/srtc.cpp $(snes)/chip/srtc/*
obj/snes-sdd1.o    : $(snes)/chip/sdd1/sdd1.cpp $(snes)/chip/sdd1/*
obj/snes-spc7110.o : $(snes)/chip/spc7110/spc7110.cpp $(snes)/chip/spc7110/*
obj/snes-obc1.o    : $(snes)/chip/obc1/obc1.cpp $(snes)/chip/obc1/*
obj/snes-st0018.o  : $(snes)/chip/st0018/st0018.cpp $(snes)/chip/st0018/*
obj/snes-msu1.o    : $(snes)/chip/msu1/msu1.cpp $(snes)/chip/msu1/*
obj/snes-link.o    : $(snes)/chip/link/link.cpp $(snes)/chip/link/*
obj/snes-sufamiturbo.o: $(snes)/chip/sufamiturbo/sufamiturbo.cpp $(snes)/chip/sufamiturbo/*

###########
# library #
###########

snes_objects := $(patsubst %,obj/%.o,$(snes_objects))

static-library: obj/libsnes.o $(snes_objects)
	ar rcs out/libsnes.a obj/libsnes.o $(snes_objects)

library: obj/libsnes.o $(snes_objects)
ifeq ($(platform),x)
	ar rcs out/libsnes.a $(snes_objects) obj/libsnes.o
	$(CXX) -o out/libsnes.so -fPIC -shared -Wl,-soname,libsnes.so.1 obj/libsnes.o $(snes_objects) -ldl -Wl,-version-script=link.T -Wl,-no-undefined
else ifeq ($(platform),xbsd)
	ar rcs out/libsnes.a $(snes_objects) obj/libsnes.o
	$(CXX) -o out/libsnes.so -fPIC -shared -Wl,-soname,libsnes.so.1 obj/libsnes.o $(snes_objects) -lc -Wl,-version-script=link.T -Wl,-no-undefined
else ifeq ($(platform),osx)
	ar rcs out/libsnes.a $(snes_objects) obj/libsnes.o
	$(CXX) -o out/libsnes.dylib -install_name $(prefix)/lib/libsnes.dylib -dynamiclib obj/libsnes.o $(snes_objects) $(link)
else ifeq ($(platform),win)
	$(CXX) -o out/snes.dll -static-libgcc -static-libstdc++ -shared -Wl,--out-implib,libsnes.a obj/libsnes.o $(snes_objects) $(link) -Wl,-version-script=link.T -Wl,-no-undefined
endif

library-install:
ifeq ($(platform),x)
	install -D -m 755 out/libsnes.a $(DESTDIR)$(prefix)/lib/libsnes.a
	install -D -m 755 out/libsnes.so $(DESTDIR)$(prefix)/lib/libsnes.so
	install -D -m 644 snes/libsnes/libsnes.hpp $(DESTDIR)$(prefix)/include/libsnes.hpp
	ldconfig -n $(DESTDIR)$(prefix)/lib
else ifeq ($(platform),osx)
	cp out/libsnes.dylib $(DESTDIR)$(prefix)/lib/libsnes.dylib
	cp snes/libsnes/libsnes.hpp $(DESTDIR)$(prefix)/include/libsnes.hpp
endif

library-uninstall:
ifeq ($(platform),x)
	rm $(DESTDIR)$(prefix)/lib/libsnes.a
	rm $(DESTDIR)$(prefix)/lib/libsnes.so
else ifeq ($(platform),osx)
	rm $(DESTDIR)$(prefix)/lib/libsnes.dylib
	rm $(DESTDIR)$(prefix)/include/libsnes.hpp
endif

