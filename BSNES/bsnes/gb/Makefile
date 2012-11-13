gb_objects := gb-interface gb-system gb-scheduler
gb_objects += gb-memory gb-cartridge
gb_objects += gb-cpu gb-ppu gb-apu
gb_objects += gb-cheat gb-video
objects += $(gb_objects)

obj/gb-interface.o: $(gb)/interface/interface.cpp $(call rwildcard,$(gb)/interface/)
obj/gb-system.o:    $(gb)/system/system.cpp $(call rwildcard,$(gb)/system/)
obj/gb-scheduler.o: $(gb)/scheduler/scheduler.cpp $(call rwildcard,$(gb)/scheduler/)
obj/gb-cartridge.o: $(gb)/cartridge/cartridge.cpp $(call rwildcard,$(gb)/cartridge/)
obj/gb-memory.o:    $(gb)/memory/memory.cpp $(call rwildcard,$(gb)/memory/)
obj/gb-cpu.o:       $(gb)/cpu/cpu.cpp $(call rwildcard,$(gb)/cpu/)
obj/gb-ppu.o:       $(gb)/ppu/ppu.cpp $(call rwildcard,$(gb)/ppu/)
obj/gb-apu.o:       $(gb)/apu/apu.cpp $(call rwildcard,$(gb)/apu/)
obj/gb-cheat.o:     $(gb)/cheat/cheat.cpp $(call rwildcard,$(gb)/cheat/)
obj/gb-video.o:     $(gb)/video/video.cpp $(call rwildcard,$(gb)/video/)
