MODULE := engines/parallaction

MODULE_OBJS := \
	adlib.o \
	balloons.o \
	callables_br.o \
	callables_ns.o \
	debug.o \
	detection.o \
	dialogue.o \
	disk.o \
	disk_br.o \
	disk_ns.o \
	exec.o \
	exec_br.o \
	exec_ns.o \
	font.o \
	gfxbase.o \
	graphics.o \
	gui.o \
	gui_br.o \
	gui_ns.o \
	input.o \
	inventory.o \
	objects.o \
	parallaction.o \
	parallaction_br.o \
	parallaction_ns.o \
	parser.o \
	parser_br.o \
	parser_ns.o \
	saveload.o \
	sound_ns.o \
	sound_br.o \
	staticres.o \
	walk.o

# This module can be built as a plugin
ifeq ($(ENABLE_PARALLACTION), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
