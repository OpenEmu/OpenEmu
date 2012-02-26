gameboy_objects := gameboy-interface gameboy-system gameboy-scheduler
gameboy_objects += gameboy-memory gameboy-cartridge
gameboy_objects += gameboy-cpu gameboy-apu gameboy-lcd
gameboy_objects += gameboy-cheat gameboy-video
objects += $(gameboy_objects)

obj/gameboy-interface.o: $(gameboy)/interface/interface_gb.cpp $(call rwildcard,$(gameboy)/interface/)
obj/gameboy-system.o: $(gameboy)/system/system_gb.cpp $(call rwildcard,$(gameboy)/system/)
obj/gameboy-scheduler.o: $(gameboy)/scheduler/scheduler_gb.cpp $(call rwildcard,$(gameboy)/scheduler/)
obj/gameboy-cartridge.o: $(gameboy)/cartridge/cartridge_gb.cpp $(call rwildcard,$(gameboy)/cartridge/)
obj/gameboy-memory.o: $(gameboy)/memory/memory_gb.cpp $(call rwildcard,$(gameboy)/memory/)
obj/gameboy-cpu.o: $(gameboy)/cpu/cpu_gb.cpp $(call rwildcard,$(gameboy)/cpu/)
obj/gameboy-apu.o: $(gameboy)/apu/apu_gb.cpp $(call rwildcard,$(gameboy)/apu/)
obj/gameboy-lcd.o: $(gameboy)/lcd/lcd_gb.cpp $(call rwildcard,$(gameboy)/lcd/)
obj/gameboy-cheat.o: $(gameboy)/cheat/cheat_gb.cpp $(call rwildcard,$(gameboy)/cheat/)
obj/gameboy-video.o: $(gameboy)/video/video_gb.cpp $(call rwildcard,$(gameboy)/video/)
