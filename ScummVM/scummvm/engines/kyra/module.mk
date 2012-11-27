MODULE := engines/kyra

MODULE_OBJS := \
	animator_lok.o \
	animator_v2.o \
	animator_hof.o \
	animator_mr.o \
	animator_tim.o \
	debugger.o \
	detection.o \
	gui.o \
	gui_v1.o \
	gui_lok.o \
	gui_v2.o \
	gui_hof.o \
	gui_mr.o \
	items_lok.o \
	items_v2.o \
	items_hof.o \
	items_mr.o \
	kyra_v1.o \
	kyra_lok.o \
	kyra_v2.o \
	kyra_hof.o \
	kyra_mr.o \
	resource.o \
	resource_intern.o \
	saveload.o \
	saveload_lok.o \
	saveload_hof.o \
	saveload_mr.o \
	scene_v1.o \
	scene_lok.o \
	scene_v2.o \
	scene_hof.o \
	scene_mr.o \
	screen.o \
	screen_lok.o \
	screen_v2.o \
	screen_hof.o \
	screen_mr.o \
	script_v1.o \
	script_lok.o \
	script_v2.o \
	script_hof.o \
	script_mr.o \
	script.o \
	script_tim.o \
	seqplayer.o \
	sequences_lok.o \
	sequences_v2.o \
	sequences_hof.o \
	sequences_mr.o \
	sound_adlib.o \
	sound_amiga.o \
	sound_digital.o \
	sound_midi.o \
	sound_pcspk.o \
	sound_towns.o \
	sound.o \
	sound_lok.o \
	sprites.o \
	staticres.o \
	text.o \
	text_lok.o \
	text_hof.o \
	text_mr.o \
	timer.o \
	timer_lok.o \
	timer_hof.o \
	timer_mr.o \
	util.o \
	vqa.o \
	wsamovie.o

KYRARPG_COMMON_OBJ = \
	gui_rpg.o \
	kyra_rpg.o \
	saveload_rpg.o \
	scene_rpg.o \
	sprites_rpg.o \
	staticres_rpg.o \
	text_rpg.o \
	timer_rpg.o

ifdef ENABLE_LOL
MODULE_OBJS += \
	$(KYRARPG_COMMON_OBJ) \
	gui_lol.o \
	items_lol.o \
	lol.o \
	saveload_lol.o \
	scene_lol.o \
	screen_lol.o \
	script_lol.o \
	sequences_lol.o \
	sound_lol.o \
	sprites_lol.o \
	staticres_lol.o \
	text_lol.o \
	timer_lol.o
endif

ifdef ENABLE_EOB
ifndef ENABLE_LOL
MODULE_OBJS += \
	$(KYRARPG_COMMON_OBJ)
endif
MODULE_OBJS += \
	chargen.o \
	eobcommon.o \
	eob.o \
	darkmoon.o \
	gui_eob.o \
	items_eob.o \
	magic_eob.o \
	saveload_eob.o \
	scene_eob.o \
	screen_eob.o \
	script_eob.o \
	sequences_eob.o \
	sequences_darkmoon.o \
	sprites_eob.o \
	staticres_eob.o \
	timer_eob.o
endif

# This module can be built as a plugin
ifeq ($(ENABLE_KYRA), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

ifeq ($(BACKEND), maemo)
#ugly workaround, screen.cpp crashes gcc version 3.4.4 (CodeSourcery ARM 2005q3-2) with anything but -O3
$(MODULE)/screen.o: $(MODULE)/screen.cpp
	$(MKDIR) $(*D)/$(DEPDIR)
	$(CXX) -Wp,-MMD,"$(*D)/$(DEPDIR)/$(*F).d",-MQ,"$@",-MP $(CXXFLAGS) -O3 $(CPPFLAGS) -c $(<) -o $*.o
endif
