MODULE := engines/lure

MODULE_OBJS := \
	animseq.o \
	debugger.o \
	decode.o \
	detection.o \
	disk.o \
	events.o \
	fights.o \
	game.o \
	hotspots.o \
	intro.o \
	lure.o \
	memory.o \
	menu.o \
	palette.o \
	res.o \
	res_struct.o \
	room.o \
	screen.o \
	scripts.o \
	sound.o \
	strings.o \
	surface.o

# This module can be built as a plugin
ifeq ($(ENABLE_LURE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
