MODULE := backends/platform/psp

MODULE_OBJS := powerman.o \
	psp_main.o \
	osys_psp.o \
	psppixelformat.o \
	memory.o \
	display_manager.o \
	display_client.o \
	default_display_client.o \
	input.o \
	cursor.o \
	trace.o \
	pspkeyboard.o \
	audio.o \
	thread.o \
	rtc.o \
	mp3.o \
	png_loader.o \
	image_viewer.o \
	tests.o \
	dummy.o

# We don't use rules.mk but rather manually update OBJS and MODULE_DIRS.
MODULE_OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS))
OBJS := $(MODULE_OBJS) $(OBJS)
MODULE_DIRS += $(sort $(dir $(MODULE_OBJS)))
