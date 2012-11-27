MODULE := engines/tsage

MODULE_OBJS := \
	blue_force/blueforce_dialogs.o \
	blue_force/blueforce_logic.o \
	blue_force/blueforce_scenes0.o \
	blue_force/blueforce_scenes1.o \
	blue_force/blueforce_scenes2.o \
	blue_force/blueforce_scenes3.o \
	blue_force/blueforce_scenes4.o \
	blue_force/blueforce_scenes5.o \
	blue_force/blueforce_scenes6.o \
	blue_force/blueforce_scenes7.o \
	blue_force/blueforce_scenes8.o \
	blue_force/blueforce_scenes9.o \
	blue_force/blueforce_speakers.o \
	converse.o \
	core.o \
	debugger.o \
	detection.o \
	dialogs.o \
	events.o \
	globals.o \
	graphics.o \
	resources.o \
	ringworld/ringworld_demo.o \
	ringworld/ringworld_dialogs.o \
	ringworld/ringworld_logic.o \
	ringworld/ringworld_scenes1.o \
	ringworld/ringworld_scenes2.o \
	ringworld/ringworld_scenes3.o \
	ringworld/ringworld_scenes4.o \
	ringworld/ringworld_scenes5.o \
	ringworld/ringworld_scenes6.o \
	ringworld/ringworld_scenes8.o \
	ringworld/ringworld_scenes10.o \
	ringworld/ringworld_speakers.o \
	ringworld2/ringworld2_dialogs.o \
	ringworld2/ringworld2_logic.o \
	ringworld2/ringworld2_scenes0.o \
	ringworld2/ringworld2_scenes1.o \
	ringworld2/ringworld2_scenes2.o \
	ringworld2/ringworld2_scenes3.o \
	ringworld2/ringworld2_speakers.o \
	saveload.o \
	scenes.o \
	sound.o \
	staticres.o \
	tsage.o \
	user_interface.o

# This module can be built as a plugin
ifeq ($(ENABLE_TSAGE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
