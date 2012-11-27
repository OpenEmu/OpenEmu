MODULE := engines/cine

MODULE_OBJS := \
	anim.o \
	bg.o \
	bg_list.o \
	console.o \
	cine.o \
	detection.o \
	gfx.o \
	main_loop.o \
	msg.o \
	object.o \
	pal.o \
	part.o \
	prc.o \
	rel.o \
	saveload.o \
	script_fw.o \
	script_os.o \
	sound.o \
	texte.o \
	unpack.o \
	various.o

# This module can be built as a plugin
ifeq ($(ENABLE_CINE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
