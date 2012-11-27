MODULE := backends/platform/wii

MODULE_OBJS := \
	main.o \
	options.o \
	osystem.o \
	osystem_gfx.o \
	osystem_sfx.o \
	osystem_events.o

# We don't use rules.mk but rather manually update OBJS and MODULE_DIRS.
MODULE_OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS))
OBJS := $(MODULE_OBJS) $(OBJS)
MODULE_DIRS += $(sort $(dir $(MODULE_OBJS)))
