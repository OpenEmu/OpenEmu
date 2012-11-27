MODULE := engines/hugo

MODULE_OBJS := \
	console.o \
	detection.o \
	dialogs.o \
	display.o \
	file.o \
	file_v1d.o \
	file_v2d.o \
	file_v3d.o \
	file_v1w.o \
	file_v2w.o \
	hugo.o \
	intro.o \
	inventory.o \
	mouse.o \
	object.o \
	object_v1d.o \
	object_v1w.o \
	object_v2d.o \
	object_v3d.o \
	parser.o \
	parser_v1w.o \
	parser_v1d.o \
	parser_v2d.o \
	parser_v3d.o \
	route.o \
	schedule.o \
	sound.o \
	text.o \
	util.o

# This module can be built as a plugin
ifeq ($(ENABLE_HUGO), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
