
MODULE := devtools/qtable

MODULE_OBJS := \
	qtable.o

# Set the name of the executable
TOOL_EXECUTABLE := qtable

# Include common rules
include $(srcdir)/rules.mk
