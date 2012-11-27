MODULE := engines/draci
 
MODULE_OBJS := \
	animation.o \
	barchive.o \
	console.o \
	detection.o \
	draci.o \
	font.o \
	game.o \
	mouse.o \
	music.o \
	saveload.o \
	screen.o \
	script.o \
	sound.o \
	sprite.o \
	surface.o \
	walking.o
 
# This module can be built as a plugin
ifeq ($(ENABLE_DRACI), DYNAMIC_PLUGIN)
PLUGIN := 1
endif
 
# Include common rules 
include $(srcdir)/rules.mk
