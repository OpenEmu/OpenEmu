MODULE := engines/cruise

MODULE_OBJS := \
	actor.o \
	background.o \
	backgroundIncrust.o \
	cell.o \
	cruise.o \
	cruise_main.o \
	ctp.o \
	dataLoader.o \
	debugger.o \
	decompiler.o \
	delphine-unpack.o \
	detection.o \
	font.o \
	function.o \
	gfxModule.o \
	linker.o \
	mainDraw.o \
	menu.o \
	mouse.o \
	object.o \
	overlay.o \
	perso.o \
	polys.o \
	saveload.o \
	script.o \
	sound.o \
	stack.o \
	staticres.o \
	various.o \
	vars.o \
	volume.o

# This module can be built as a plugin
ifeq ($(ENABLE_CRUISE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
