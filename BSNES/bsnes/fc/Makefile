fc_objects := fc-interface fc-system fc-scheduler fc-input
fc_objects += fc-memory fc-cartridge fc-cpu fc-apu fc-ppu
fc_objects += fc-cheat fc-video
objects += $(fc_objects)

obj/fc-interface.o: $(fc)/interface/interface.cpp $(call rwildcard,$(fc)/interface/)
obj/fc-system.o: $(fc)/system/system.cpp $(call rwildcard,$(fc)/system/)
obj/fc-scheduler.o: $(fc)/scheduler/scheduler.cpp $(call rwildcard,$(fc)/scheduler/)
obj/fc-input.o: $(fc)/input/input.cpp $(call rwildcard,$(fc)/input/)
obj/fc-memory.o: $(fc)/memory/memory.cpp $(call rwildcard,$(fc)/memory/)
obj/fc-cartridge.o: $(fc)/cartridge/cartridge.cpp $(call rwildcard,$(fc)/cartridge/)
obj/fc-cpu.o: $(fc)/cpu/cpu.cpp $(call rwildcard,$(fc)/cpu/)
obj/fc-apu.o: $(fc)/apu/apu.cpp $(call rwildcard,$(fc)/apu/)
obj/fc-ppu.o: $(fc)/ppu/ppu.cpp $(call rwildcard,$(fc)/ppu/)
obj/fc-cheat.o: $(fc)/cheat/cheat.cpp $(call rwildcard,$(fc)/cheat/)
obj/fc-video.o: $(fc)/video/video.cpp $(call rwildcard,$(fc)/video/)
