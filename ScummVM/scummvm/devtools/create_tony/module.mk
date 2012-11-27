MODULE := devtools/create_tony

MODULE_OBJS := \
	create_tony.o

# Set the name of the executable
TOOL_EXECUTABLE := create_tony

# Include common rules
include $(srcdir)/rules.mk
