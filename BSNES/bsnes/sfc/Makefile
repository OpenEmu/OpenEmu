sfc_objects := sfc-interface sfc-system sfc-controller
sfc_objects += sfc-cartridge sfc-cheat
sfc_objects += sfc-memory sfc-cpu sfc-smp sfc-dsp sfc-ppu
sfc_objects += sfc-icd2 sfc-nss sfc-superfx sfc-sa1
sfc_objects += sfc-necdsp sfc-hitachidsp sfc-armdsp
sfc_objects += sfc-bsx sfc-srtc sfc-sdd1 sfc-spc7110
sfc_objects += sfc-obc1 sfc-sufamiturbo
sfc_objects += sfc-msu1 sfc-link
objects += $(sfc_objects)

ifeq ($(profile),accuracy)
  flags += -DPROFILE_ACCURACY
  sfccpu := $(sfc)/cpu
  sfcsmp := $(sfc)/smp
  sfcdsp := $(sfc)/dsp
  sfcppu := $(sfc)/ppu
else ifeq ($(profile),compatibility)
  flags += -DPROFILE_COMPATIBILITY
  sfccpu := $(sfc)/cpu
  sfcsmp := $(sfc)/smp
  sfcdsp := $(sfc)/alt/dsp
  sfcppu := $(sfc)/alt/ppu-compatibility
else ifeq ($(profile),performance)
  flags += -DPROFILE_PERFORMANCE
  sfccpu := $(sfc)/alt/cpu
  sfcsmp := $(sfc)/alt/smp
  sfcdsp := $(sfc)/alt/dsp
  sfcppu := $(sfc)/alt/ppu-performance
endif

obj/sfc-interface.o : $(sfc)/interface/interface.cpp $(call rwildcard,$(sfc)/interface)
obj/sfc-system.o    : $(sfc)/system/system.cpp $(call rwildcard,$(sfc)/system/)
obj/sfc-controller.o: $(sfc)/controller/controller.cpp $(call rwildcard,$(sfc)/controller/)
obj/sfc-memory.o    : $(sfc)/memory/memory.cpp $(call rwildcard,$(sfc)/memory/)
obj/sfc-cpu.o       : $(sfccpu)/cpu.cpp $(call rwildcard,$(sfccpu)/)
obj/sfc-smp.o       : $(sfcsmp)/smp.cpp $(call rwildcard,$(sfcsmp)/)
obj/sfc-dsp.o       : $(sfcdsp)/dsp.cpp $(call rwildcard,$(sfcdsp)/)
obj/sfc-ppu.o       : $(sfcppu)/ppu.cpp $(call rwildcard,$(sfcppu)/)
obj/sfc-cartridge.o : $(sfc)/cartridge/cartridge.cpp $(sfc)/cartridge/*
obj/sfc-cheat.o     : $(sfc)/cheat/cheat.cpp $(sfc)/cheat/*

obj/sfc-nss.o        : $(sfc)/chip/nss/nss.cpp $(call rwildcard,$(sfc)/chip/nss/)
obj/sfc-icd2.o       : $(sfc)/chip/icd2/icd2.cpp $(call rwildcard,$(sfc)/chip/icd2/)
obj/sfc-superfx.o    : $(sfc)/chip/superfx/superfx.cpp $(call rwildcard,$(sfc)/chip/superfx/)
obj/sfc-sa1.o        : $(sfc)/chip/sa1/sa1.cpp $(call rwildcard,$(sfc)/chip/sa1/)
obj/sfc-necdsp.o     : $(sfc)/chip/necdsp/necdsp.cpp $(call rwildcard,$(sfc)/chip/necdsp/)
obj/sfc-hitachidsp.o : $(sfc)/chip/hitachidsp/hitachidsp.cpp $(call rwildcard,$(sfc)/chip/hitachidsp/)
obj/sfc-armdsp.o     : $(sfc)/chip/armdsp/armdsp.cpp $(call rwildcard,$(sfc)/chip/armdsp/)
obj/sfc-bsx.o        : $(sfc)/chip/bsx/bsx.cpp $(call rwildcard,$(sfc)/chip/bsx/)
obj/sfc-srtc.o       : $(sfc)/chip/srtc/srtc.cpp $(sfc)/chip/srtc/*
obj/sfc-sdd1.o       : $(sfc)/chip/sdd1/sdd1.cpp $(sfc)/chip/sdd1/*
obj/sfc-spc7110.o    : $(sfc)/chip/spc7110/spc7110.cpp $(sfc)/chip/spc7110/*
obj/sfc-obc1.o       : $(sfc)/chip/obc1/obc1.cpp $(sfc)/chip/obc1/*
obj/sfc-sufamiturbo.o: $(sfc)/chip/sufamiturbo/sufamiturbo.cpp $(sfc)/chip/sufamiturbo/*
obj/sfc-msu1.o       : $(sfc)/chip/msu1/msu1.cpp $(sfc)/chip/msu1/*
obj/sfc-link.o       : $(sfc)/chip/link/link.cpp $(sfc)/chip/link/*
