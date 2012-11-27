
MODULE := devtools/create_project

MODULE_OBJS := \
	create_project.o \
	codeblocks.o \
	msvc.o \
	visualstudio.o \
	msbuild.o \
	xcode.o

# Set the name of the executable
TOOL_EXECUTABLE := create_project

# Set custom build flags for create_project.o: It uses C++ iostreams,
# which make use of global constructors. So we don't want warnings for
# that.
$(srcdir)/devtools/create_project/create_project.o: CXXFLAGS:=$(filter-out -Wglobal-constructors,$(CXXFLAGS))

# Include common rules
include $(srcdir)/rules.mk

# Silence variadic macros warning for C++ (disabled as this is included globally)
#CXXFLAGS := $(CXXFLAGS) -Wno-variadic-macros
