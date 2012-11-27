MODULE := engines/teenagent

MODULE_OBJS := \
	actor.o \
	animation.o \
	callbacks.o \
	console.o \
	detection.o \
	dialog.o \
	font.o \
	inventory.o \
	music.o \
	objects.o \
	pack.o \
	resources.o \
	scene.o \
	segment.o \
	surface.o \
	surface_list.o \
	teenagent.o

# This module can be built as a plugin
ifeq ($(ENABLE_TEENAGENT), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
