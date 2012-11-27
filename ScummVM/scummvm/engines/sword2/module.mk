MODULE := engines/sword2

MODULE_OBJS := \
	animation.o \
	anims.o \
	console.o \
	controls.o \
	debug.o \
	events.o \
	function.o \
	header.o \
	icons.o \
	interpreter.o \
	layers.o \
	logic.o \
	maketext.o \
	memory.o \
	menu.o \
	mouse.o \
	music.o \
	palette.o \
	protocol.o \
	render.o \
	resman.o \
	router.o \
	saveload.o \
	screen.o \
	scroll.o \
	sound.o \
	speech.o \
	sprite.o \
	startup.o \
	sword2.o \
	sync.o \
	walker.o

# This module can be built as a plugin
ifeq ($(ENABLE_SWORD2), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
