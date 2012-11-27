MODULE := backends/platform/iphone

MODULE_OBJS := \
	osys_main.o \
	osys_events.o \
	osys_sound.o \
	osys_video.o \
	iphone_main.o \
	iphone_video.o \
	iphone_keyboard.o

# We don't use rules.mk but rather manually update OBJS and MODULE_DIRS.
MODULE_OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS))
OBJS := $(MODULE_OBJS) $(OBJS)
MODULE_DIRS += $(sort $(dir $(MODULE_OBJS)))
