MODULE := engines/touche

MODULE_OBJS := \
	console.o \
	detection.o \
	graphics.o \
	menu.o \
	midi.o \
	opcodes.o \
	resource.o \
	saveload.o \
	staticres.o \
	touche.o

# This module can be built as a plugin
ifeq ($(ENABLE_TOUCHE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
