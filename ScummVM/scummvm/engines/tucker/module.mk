MODULE := engines/tucker

MODULE_OBJS := \
	console.o \
	detection.o \
	graphics.o \
	locations.o \
	resource.o \
	saveload.o \
	sequences.o \
	staticres.o \
	tucker.o

# This module can be built as a plugin
ifeq ($(ENABLE_TUCKER), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
