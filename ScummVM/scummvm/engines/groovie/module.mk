MODULE := engines/groovie

MODULE_OBJS := \
	cell.o \
	cursor.o \
	debug.o \
	detection.o \
	font.o \
	graphics.o \
	groovie.o \
	lzss.o \
	music.o \
	player.o \
	resource.o \
	roq.o \
	saveload.o \
	script.o \
	vdx.o

# This module can be built as a plugin
ifeq ($(ENABLE_GROOVIE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
