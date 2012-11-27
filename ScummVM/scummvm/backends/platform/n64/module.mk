MODULE := backends/platform/n64

MODULE_OBJS := \
	nintendo64.o \
	osys_n64_base.o \
	osys_n64_events.o \
	osys_n64_utilities.o \
	pakfs_save_manager.o \
	framfs_save_manager.o

# We don't use rules.mk but rather manually update OBJS and MODULE_DIRS.
MODULE_OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS))
OBJS := $(MODULE_OBJS) $(OBJS)
MODULE_DIRS += $(sort $(dir $(MODULE_OBJS)))
