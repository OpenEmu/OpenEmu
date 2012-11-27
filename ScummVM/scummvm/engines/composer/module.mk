MODULE := engines/composer

MODULE_OBJS = \
	composer.o \
	detection.o \
	graphics.o \
	resource.o \
	scripting.o

# This module can be built as a plugin
ifeq ($(ENABLE_COMPOSER), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
