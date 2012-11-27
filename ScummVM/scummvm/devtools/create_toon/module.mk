MODULE := devtools/create_toon

MODULE_OBJS := \
	create_toon.o

# Set the name of the executable
TOOL_EXECUTABLE := create_toon

# Include common rules
include $(srcdir)/rules.mk
