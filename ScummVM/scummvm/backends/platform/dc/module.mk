MODULE := backends/platform/dc

MODULE_OBJS :=	dcmain.o time.o display.o audio.o input.o selector.o icon.o \
	label.o vmsave.o softkbd.o dcloader.o cache.o dc-fs.o plugins.o

# We don't use rules.mk but rather manually update OBJS and MODULE_DIRS.
MODULE_OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS))
OBJS := $(MODULE_OBJS) $(OBJS)
MODULE_DIRS += $(sort $(dir $(MODULE_OBJS)))
