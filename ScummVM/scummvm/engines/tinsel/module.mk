MODULE := engines/tinsel

MODULE_OBJS := \
	actors.o \
	adpcm.o \
	anim.o \
	background.o \
	bg.o \
	bmv.o \
	cliprect.o \
	config.o \
	cursor.o \
	debugger.o \
	detection.o \
	dialogs.o \
	drives.o \
	effect.o \
	events.o \
	faders.o \
	font.o \
	graphics.o \
	handle.o \
	heapmem.o \
	mareels.o \
	move.o \
	multiobj.o \
	music.o \
	object.o \
	palette.o \
	pcode.o \
	pdisplay.o \
	play.o \
	polygons.o \
	rince.o \
	saveload.o \
	savescn.o \
	scene.o \
	sched.o \
	scn.o \
	scroll.o \
	sound.o \
	strres.o \
	sysvar.o \
	text.o \
	timers.o \
	tinlib.o \
	tinsel.o \
	token.o

# This module can be built as a plugin
ifeq ($(ENABLE_TINSEL), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
