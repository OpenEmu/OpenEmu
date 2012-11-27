
TOOLPATH = /opt/mips64-toolchain
LIBN64PATH = $(TOOLPATH)/hkz-libn64/
GCCN64PREFIX = $(TOOLPATH)/bin/mips64-

srcdir = ../../..
VPATH = $(srcdir)

CC = $(GCCN64PREFIX)gcc
CXX = $(GCCN64PREFIX)g++
AS = $(GCCN64PREFIX)as
LD = $(GCCN64PREFIX)g++
OBJCOPY = $(GCCN64PREFIX)objcopy
AR = $(GCCN64PREFIX)ar cru
RANLIB = $(GCCN64PREFIX)ranlib

DEFINES += -D__N64__ -DLIMIT_FPS -DNONSTANDARD_PORT -DDISABLE_DEFAULT_SAVEFILEMANAGER -DDISABLE_TEXT_CONSOLE -DDISABLE_COMMAND_LINE -DDISABLE_FANCY_THEMES -DDISABLE_DOSBOX_OPL -DENABLE_VKEYBD -DUSE_ZLIB
LIBS += -lpakfs -lframfs -ln64 -ln64utils -lromfs

#DEFINES += -D_ENABLE_DEBUG_

USE_LIBMAD=0
USE_LIBOGG=1

ifeq ($(USE_LIBMAD),1)
DEFINES += -DUSE_MAD
LIBS	+= -lmad
endif
ifeq ($(USE_LIBOGG), 1)
DEFINES += -DUSE_VORBIS -DUSE_TREMOR
LIBS	+= -lvorbisidec
endif

LIBS += -lm -lstdc++ -lc -lgcc -lz -lnosys

CXXFLAGS = -g -mno-extern-sdata -O2 --param max-inline-insns-auto=20 -fomit-frame-pointer -march=vr4300 -mtune=vr4300 -mhard-float -fno-rtti -fno-exceptions -Wno-multichar -Wshadow -I$(LIBN64PATH) -I$(TOOLPATH)/include -I./ -I$(srcdir) -I$(srcdir)/engines
LDFLAGS = -g -march=vr4300 -mtune=vr4300 -nodefaultlibs -nostartfiles -mno-crt0 -L$(LIBN64PATH) -L$(TOOLPATH)/lib $(LIBS) -T n64ld_cpp.x -Xlinker -Map -Xlinker scummvm.map

TARGET = scummvm
DEPDIR = .deps
CXX_UPDATE_DEP_FLAG = -Wp,-MMD,"$(*D)/$(DEPDIR)/$(*F).d",-MQ,"$@",-MP
MKDIR = mkdir -p
RM = rm -f
RM_REC = rm -rf

VERBOSE_BUILD=1
HAVE_GCC3=1
USE_RGB_COLOR=0

ENABLED=STATIC_PLUGIN

#ENABLE_SCUMM = $(ENABLED)
#ENABLE_SCI = $(ENABLED)
#ENABLE_GOB = $(ENABLED)
#ENABLE_PARALLACTION = $(ENABLED)
#ENABLE_KYRA = $(ENABLED)
#ENABLE_AGOS = $(ENABLED)
#ENABLE_AGI = $(ENABLED)
#ENABLE_QUEEN = $(ENABLED)
#ENABLE_MADE = $(ENABLED)
#ENABLE_SAGA = $(ENABLED)
#ENABLE_TEENAGENT = $(ENABLED)
#ENABLE_DRACI = $(ENABLED)

OBJS :=	nintendo64.o osys_n64_base.o osys_n64_events.o osys_n64_utilities.o pakfs_save_manager.o framfs_save_manager.o

BACKEND := n64

include $(srcdir)/Makefile.common

MODULE_DIRS += ./

all: $(TARGET).v64

$(TARGET).v64: $(TARGET).bin ROMFS.img bootcode
	cat bootcode $(TARGET).bin ROMFS.img > $(TARGET).v64
	./pad_rom.sh $(TARGET).v64

ROMFS.img:
	genromfs -f ./ROMFS.img -d ./ROMFS -V romtest

$(TARGET).elf: $(OBJS)
	$(LD) -o $(TARGET).elf $(OBJS) $(LDFLAGS)

$(TARGET).bin : $(TARGET).elf
	$(OBJCOPY) $(TARGET).elf $(TARGET).bin -O binary

spotless : distclean
	$(RM) *.bin *.elf *.v64 *.img *.bak *.tmp *.map

send: $(TARGET).v64
	sudo ucon64 --xv64  $(TARGET).v64
