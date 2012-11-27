MODULE := devtools/create_hugo

MODULE_OBJS := \
	create_hugo.o

# Set the name of the executable
TOOL_EXECUTABLE := create_hugo

# Include common rules
include $(srcdir)/rules.mk
