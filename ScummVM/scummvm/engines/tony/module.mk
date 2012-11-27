MODULE := engines/tony

MODULE_OBJS := \
	custom.o \
	debugger.o \
	detection.o \
	font.o \
	game.o \
	gfxcore.o \
	gfxengine.o \
	globals.o \
	input.o \
	inventory.o \
	loc.o \
	sound.o \
	tony.o \
	tonychar.o \
	utils.o \
	window.o \
	mpal/expr.o \
	mpal/loadmpc.o \
	mpal/memory.o \
	mpal/mpal.o \
	mpal/mpalutils.o \
	mpal/lzo.o

# This module can be built as a plugin
ifeq ($(ENABLE_TONY), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
