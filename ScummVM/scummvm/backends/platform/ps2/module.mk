MODULE := backends/platform/ps2

MODULE_OBJS := \
	DmaPipe.o \
    Gs2dScreen.o \
    irxboot.o \
	ps2input.o \
	ps2pad.o \
	savefilemgr.o \
    fileio.o \
    asyncfio.o \
	icon.o \
    cd.o \
    eecodyvdfs.o \
    rpckbd.o \
    systemps2.o \
    ps2mutex.o \
    ps2time.o \
	ps2debug.o

# We don't use rules.mk but rather manually update OBJS and MODULE_DIRS.
MODULE_OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS))
OBJS := $(MODULE_OBJS) $(OBJS)
MODULE_DIRS += $(sort $(dir $(MODULE_OBJS)))
