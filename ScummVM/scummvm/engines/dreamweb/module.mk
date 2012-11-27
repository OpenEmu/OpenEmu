MODULE := engines/dreamweb

MODULE_OBJS := \
	backdrop.o \
	console.o \
	detection.o \
	dreamweb.o \
	keypad.o \
	monitor.o \
	mouse.o \
	newplace.o \
	object.o \
	pathfind.o \
	people.o \
	print.o \
	rain.o \
	saveload.o \
	sound.o \
	sprite.o \
	stubs.o \
	talk.o \
	titles.o \
	use.o \
	vgafades.o \
	vgagrafx.o

# This module can be built as a plugin
ifeq ($(ENABLE_DREAMWEB), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
