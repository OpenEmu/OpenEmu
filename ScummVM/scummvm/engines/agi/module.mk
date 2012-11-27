MODULE := engines/agi

MODULE_OBJS := \
	agi.o \
	checks.o \
	console.o \
	cycle.o \
	detection.o \
	global.o \
	graphics.o \
	id.o \
	inv.o \
	keyboard.o \
	loader_v1.o \
	loader_v2.o \
	loader_v3.o \
	logic.o \
	lzw.o \
	menu.o \
	motion.o \
	objects.o \
	opcodes.o \
	op_cmd.o \
	op_dbg.o \
	op_test.o \
	picture.o \
	preagi.o \
	preagi_mickey.o \
	preagi_troll.o \
	preagi_winnie.o \
	saveload.o \
	sound.o \
	sound_2gs.o \
	sound_coco3.o \
	sound_midi.o \
	sound_pcjr.o \
	sound_sarien.o \
	sprite.o \
	text.o \
	view.o \
	wagparser.o \
	words.o


# This module can be built as a plugin
ifeq ($(ENABLE_AGI), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
