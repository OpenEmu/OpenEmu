
MODULE := devtools/skycpt

MODULE_OBJS := \
	AsciiCptCompile.o \
	KmpSearch.o \
	TextFile.o \
	cptcompiler.o \
	cpthelp.o \
	idFinder.o

# Set the name of the executable
TOOL_EXECUTABLE := skycpt

# Include common rules
include $(srcdir)/rules.mk
