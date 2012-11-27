MODULE := engines/scumm

MODULE_OBJS := \
	actor.o \
	akos.o \
	base-costume.o \
	bomp.o \
	boxes.o \
	camera.o \
	charset.o \
	charset-fontdata.o \
	costume.o \
	cursor.o \
	debugger.o \
	detection.o \
	dialogs.o \
	file.o \
	file_nes.o \
	gfx_towns.o \
	gfx.o \
	he/resource_he.o \
	he/script_v60he.o \
	he/script_v70he.o \
	he/sound_he.o \
	help.o \
	imuse/imuse.o \
	imuse/imuse_part.o \
	imuse/imuse_player.o \
	imuse/instrument.o \
	imuse/mac_m68k.o \
	imuse/pcspk.o \
	imuse/sysex_samnmax.o \
	imuse/sysex_scumm.o \
	input.o \
	midiparser_ro.o \
	object.o \
	palette.o \
	player_apple2.o \
	player_mod.o \
	player_nes.o \
	player_pce.o \
	player_sid.o \
	player_towns.o \
	player_v1.o \
	player_v2.o \
	player_v2a.o \
	player_v2base.o \
	player_v2cms.o \
	player_v3a.o \
	player_v4a.o \
	resource_v2.o \
	resource_v3.o \
	resource_v4.o \
	resource.o \
	room.o \
	saveload.o \
	script_v0.o \
	script_v2.o \
	script_v3.o \
	script_v4.o \
	script_v5.o \
	script_v6.o \
	script.o \
	scumm.o \
	sound.o \
	string.o \
	usage_bits.o \
	util.o \
	vars.o \
	verbs.o

ifdef USE_ARM_COSTUME_ASM
MODULE_OBJS += \
	proc3ARM.o
endif

ifdef ENABLE_SCUMM_7_8
MODULE_OBJS += \
	nut_renderer.o \
	script_v8.o \
	imuse_digi/dimuse.o \
	imuse_digi/dimuse_bndmgr.o \
	imuse_digi/dimuse_codecs.o \
	imuse_digi/dimuse_music.o \
	imuse_digi/dimuse_sndmgr.o \
	imuse_digi/dimuse_script.o \
	imuse_digi/dimuse_track.o \
	imuse_digi/dimuse_tables.o \
	insane/insane.o \
	insane/insane_ben.o \
	insane/insane_enemy.o \
	insane/insane_scenes.o \
	insane/insane_iact.o \
	smush/channel.o \
	smush/codec1.o \
	smush/codec37.o \
	smush/codec47.o \
	smush/imuse_channel.o \
	smush/smush_player.o \
	smush/saud_channel.o \
	smush/smush_mixer.o \
	smush/smush_font.o

ifdef USE_ARM_SMUSH_ASM
MODULE_OBJS += \
	smush/codec47ARM.o
endif

endif

ifdef USE_ARM_GFX_ASM
MODULE_OBJS += \
	gfxARM.o
endif

ifdef ENABLE_HE
MODULE_OBJS += \
	he/animation_he.o \
	he/cup_player_he.o \
	he/floodfill_he.o \
	he/logic_he.o \
	he/palette_he.o \
	he/script_v71he.o \
	he/script_v72he.o \
	he/script_v80he.o \
	he/script_v90he.o \
	he/script_v100he.o \
	he/sprite_he.o \
	he/wiz_he.o \
	he/logic/baseball2001.o \
	he/logic/basketball.o \
	he/logic/football.o \
	he/logic/funshop.o \
	he/logic/moonbase.o \
	he/logic/puttrace.o \
	he/logic/soccer.o
endif

# This module can be built as a plugin
ifeq ($(ENABLE_SCUMM), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
