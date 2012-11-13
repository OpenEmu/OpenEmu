gba_objects := gba-memory gba-interface gba-scheduler gba-system
gba_objects += gba-video gba-cartridge
gba_objects += gba-cpu gba-ppu gba-apu
objects += $(gba_objects)

obj/gba-memory.o:    $(gba)/memory/memory.cpp $(call rwildcard,$(gba)/memory)
obj/gba-interface.o: $(gba)/interface/interface.cpp $(call rwildcard,$(gba)/interface)
obj/gba-scheduler.o: $(gba)/scheduler/scheduler.cpp $(call rwildcard,$(gba)/scheduler)
obj/gba-system.o:    $(gba)/system/system.cpp $(call rwildcard,$(gba)/system)
obj/gba-video.o:     $(gba)/video/video.cpp $(call rwildcard,$(gba)/video)
obj/gba-cartridge.o: $(gba)/cartridge/cartridge.cpp $(call rwildcard,$(gba)/cartridge)
obj/gba-cpu.o:       $(gba)/cpu/cpu.cpp $(call rwildcard,$(gba)/cpu)
obj/gba-ppu.o:       $(gba)/ppu/ppu.cpp $(call rwildcard,$(gba)/ppu)
obj/gba-apu.o:       $(gba)/apu/apu.cpp $(call rwildcard,$(gba)/apu)
