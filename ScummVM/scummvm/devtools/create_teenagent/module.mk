
MODULE := devtools/create_teenagent

MODULE_OBJS := \
	create_teenagent.o \
	util.o

# Set the name of the executable
TOOL_EXECUTABLE := create_teenagent

# Include common rules
include $(srcdir)/rules.mk
