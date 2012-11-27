MODULE := backends/platform/sdl

MODULE_OBJS := \
	sdl.o

ifdef POSIX
MODULE_OBJS += \
	posix/posix-main.o \
	posix/posix.o
endif

ifdef MACOSX
MODULE_OBJS += \
	macosx/macosx-main.o \
	macosx/macosx.o \
	macosx/appmenu_osx.o
endif

ifdef WIN32
MODULE_OBJS += \
	win32/win32-main.o \
	win32/win32.o
endif

ifdef AMIGAOS
MODULE_OBJS += \
	amigaos/amigaos-main.o \
	amigaos/amigaos.o
endif

ifdef PLAYSTATION3
MODULE_OBJS += \
	ps3/ps3-main.o \
	ps3/ps3.o
endif

# We don't use rules.mk but rather manually update OBJS and MODULE_DIRS.
MODULE_OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS))
OBJS := $(MODULE_OBJS) $(OBJS)
MODULE_DIRS += $(sort $(dir $(MODULE_OBJS)))
