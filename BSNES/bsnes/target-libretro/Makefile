processors := arm hg51b upd96050 gsu r65816 spc700 lr35902
include processor/Makefile

include sfc/Makefile
include gb/Makefile
output := libretro

ifeq ($(platform),x)
  flags += -fPIC
else ifeq ($(platform),osx)
  flags += -fPIC
else ifeq ($(platform),win)
endif

#rules
objects += libretro
objects := $(patsubst %,obj/%.o,$(objects))

obj/libretro.o: $(ui)/libretro.cpp $(ui)/*

#targets
build: $(objects)
ifeq ($(platform),x)
	$(cpp) -o out/libretro.so -shared $(objects) -ldl -Wl,--no-undefined -Wl,--version-script=$(ui)/link.T
else ifeq ($(platform),osx)
	$(cpp) -o out/libretro.dylib -dynamiclib $(objects)
else ifeq ($(platform),win)
	$(cpp) -o out/retro.dll -shared $(objects) -Wl,--no-undefined -Wl,--version-script=$(ui)/link.T -static-libgcc -static-libstdc++
endif

install:
ifeq ($(platform),x)
	install -D -m 755 out/libretro.so $(DESTDIR)$(prefix)/lib/libretro.so
else ifeq ($(platform),osx)
	cp out/libretro.dylib $(DESTDIR)$(prefix)/lib/libretro.dylib
endif

uninstall:
ifeq ($(platform),x)
	rm $(DESTDIR)$(prefix)/lib/libretro.so
else ifeq ($(platform),osx)
	rm /usr/local/lib/libretro.dylib
endif

