MODULE := engines/cge

MODULE_OBJS := \
	bitmap.o \
	cge.o \
	cge_main.o \
	console.o \
	detection.o \
	events.o \
	fileio.o \
	game.o \
	snail.o \
	sound.o \
	talk.o \
	text.o \
	vga13h.o \
	vmenu.o \
	walk.o

MODULE_DIRS += \
	engines/cge

# This module can be built as a plugin
ifeq ($(ENABLE_CGE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules 
include $(srcdir)/rules.mk

