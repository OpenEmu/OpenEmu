
MODULE := devtools/create_kyradat

MODULE_OBJS := \
	create_kyradat.o \
	extract.o \
	games.o \
	md5.o \
	pak.o \
	search.o \
	tables.o \
	util.o

# Set the name of the executable
TOOL_EXECUTABLE := create_kyradat

# Include common rules
include $(srcdir)/rules.mk
