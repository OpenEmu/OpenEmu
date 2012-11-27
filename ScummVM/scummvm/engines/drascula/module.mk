MODULE := engines/drascula

MODULE_OBJS := \
	actors.o \
	animation.o \
	console.o \
	converse.o \
	detection.o \
	drascula.o \
	graphics.o \
	interface.o \
	objects.o \
	palette.o \
	resource.o \
	rooms.o \
	saveload.o \
	sound.o \
	talk.o


# This module can be built as a plugin
ifeq ($(ENABLE_DRASCULA), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
