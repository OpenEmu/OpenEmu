
MODULE := devtools/create_lure

MODULE_OBJS := \
	create_lure_dat.o \
	process_actions.o

# Set the name of the executable
TOOL_EXECUTABLE := create_lure

# Include common rules
include $(srcdir)/rules.mk
