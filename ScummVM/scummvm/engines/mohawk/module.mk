MODULE := engines/mohawk

MODULE_OBJS = \
	bitmap.o \
	console.o \
	cursors.o \
	detection.o \
	dialogs.o \
	graphics.o \
	installer_archive.o \
	livingbooks.o \
	livingbooks_code.o \
	livingbooks_graphics.o \
	livingbooks_lbx.o \
	mohawk.o \
	resource.o \
	sound.o \
	video.o \
	view.o

ifdef ENABLE_CSTIME
MODULE_OBJS += \
	cstime.o \
	cstime_cases.o \
	cstime_game.o \
	cstime_graphics.o \
	cstime_ui.o \
	cstime_view.o
endif

ifdef ENABLE_MYST
MODULE_OBJS += \
	myst.o \
	myst_areas.o \
	myst_graphics.o \
	myst_scripts.o \
	myst_state.o \
	resource_cache.o \
	myst_stacks/channelwood.o \
	myst_stacks/credits.o \
	myst_stacks/demo.o \
	myst_stacks/dni.o \
	myst_stacks/intro.o \
	myst_stacks/makingof.o \
	myst_stacks/mechanical.o \
	myst_stacks/myst.o \
	myst_stacks/preview.o \
	myst_stacks/selenitic.o \
	myst_stacks/slides.o \
	myst_stacks/stoneship.o
endif

ifdef ENABLE_RIVEN
MODULE_OBJS += \
	riven.o \
	riven_external.o \
	riven_graphics.o \
	riven_saveload.o \
	riven_scripts.o \
	riven_vars.o
endif

# This module can be built as a plugin
ifeq ($(ENABLE_MOHAWK), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
