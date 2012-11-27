
MODULE := devtools/create_drascula

MODULE_OBJS := \
	create_drascula.o

# Set the name of the executable
TOOL_EXECUTABLE := create_drascula

# Include common rules
include $(srcdir)/rules.mk
