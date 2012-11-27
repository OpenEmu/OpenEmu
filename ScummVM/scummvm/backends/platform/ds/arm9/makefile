srcdir      ?= .
DEPDIR      := .deps

#DYNAMIC_MODULES = 1
libndsdir = $(DEVKITPRO)/libnds
#libndsdir = /home/neil/devkitpror21/libnds

# Select the build by setting SCUMM_BUILD to a,b,c,d,e,f or g.
# Anything else gets build a.

ifeq ($(SCUMM_BUILD),k)
	DS_BUILD_K = 1
else
 ifeq ($(SCUMM_BUILD),j)
 	DS_BUILD_J = 1
 else
  ifeq ($(SCUMM_BUILD),i)
        DS_BUILD_I = 1
  else
   ifeq ($(SCUMM_BUILD),h)
          DS_BUILD_H = 1
   else
    ifeq ($(SCUMM_BUILD),g)
         DS_BUILD_G = 1
    else
     ifeq ($(SCUMM_BUILD),f)
         DS_BUILD_F = 1
     else
      ifeq ($(SCUMM_BUILD),e)
         DS_BUILD_E = 1
      else
       ifeq ($(SCUMM_BUILD),d)
         DS_BUILD_D = 1
       else
        ifeq ($(SCUMM_BUILD),c)
         DS_BUILD_C = 1
        else
         ifeq ($(SCUMM_BUILD),b)
         DS_BUILD_B = 1
         else
          DS_BUILD_A = 1
         endif
        endif
       endif
      endif
     endif
    endif
   endif
  endif
 endif
endif
# To do:
# - FAT cache?



# Uncomment the following line to build in support for MP3 audio
# using libmad:

ifdef DS_BUILD_F
	# TODO: Fix this.  When libmad is compiled in, the Kyrandia resource loading
	# searches through it's entire index to find an mp3 each time a voice sample is requested
	# this causes a nasty pause.
else
	ifdef DS_BUILD_E
	# TODO: Inherit the earth uses so much RAM that I have removed libmad in order to
	# claw some back.



	else
		ifdef DS_BUILD_I

		else
			ifdef DS_BUILD_K

			else
				USE_MAD = 1
			endif
		endif
	endif
endif

# Uncomment the following line to enable support for the
# ace DS Debugger (remembering to make the same change in the arm7 makefile):
#USE_DEBUGGER = 1

# Uncomment the following line to enable the profiler
#USE_PROFILER = 1

# NOTE: The header and libs for the debugger is assumed to be in the libnds
# folder.

vpath %.h $(srcdir)
vpath %.cpp $(srcdir)
vpath %.c $(srcdir)
vpath %.m $(srcdir)
vpath %.asm $(srcdir)
vpath %.s $(srcdir)

# Command to build libmad is:
# ./configure --host=arm-elf --enable-speed --enable-sso -enable-fpm=arm CFLAGS='-specs=ds_arm9.specs -mthumb-interwork'
#
# I actually had to use
# ./configure --host=arm-elf --enable-speed --enable-sso -enable-fpm=arm CFLAGS='-specs=ds_arm9.specs -mthumb-interwork' LDFLAGS='C:/Progra~1/devkitpro/libnds/lib/libnds9.a' --disable-shared --disable-debugging


# handy command to find where the big symbols are in the ELF:
# arm-eabi-nm -S --radix=d --demangle scummvm.elf |sort -n -r --key=2 |less

ARM = 1
USE_ARM_SOUND_ASM = 1
USE_ARM_COSTUME_ASM = 1
#WRAP_MALLOC = 1

ifdef DS_BUILD_A
	DEFINES = -DDS_BUILD_A -DUSE_ARM_GFX_ASM -DUSE_ARM_COSTUME_ASM
	LOGO = logoa.bmp
	ENABLE_SCUMM = STATIC_PLUGIN
	USE_ARM_GFX_ASM = 1
	BUILD=scummvm-A
endif

ifdef DS_BUILD_B
	DEFINES = -DDS_BUILD_B
	LOGO = logob.bmp
	ENABLE_SKY = STATIC_PLUGIN
	ENABLE_QUEEN = STATIC_PLUGIN
	BUILD=scummvm-B
endif

ifdef DS_BUILD_C
	DEFINES = -DDS_BUILD_C
	LOGO = logoc.bmp
	ENABLE_AGOS = STATIC_PLUGIN
	BUILD=scummvm-C
endif

ifdef DS_BUILD_D
	DEFINES = -DDS_BUILD_D
	LOGO = logod.bmp
	ENABLE_GOB = STATIC_PLUGIN
	ENABLE_CINE = STATIC_PLUGIN
	ENABLE_AGI = STATIC_PLUGIN
	BUILD=scummvm-D
endif

ifdef DS_BUILD_E
	DEFINES = -DDS_BUILD_E
	LOGO = logoe.bmp
	ENABLE_SAGA = STATIC_PLUGIN
	BUILD=scummvm-E
endif

ifdef DS_BUILD_F
	DEFINES = -DDS_BUILD_F
	LOGO = logof.bmp
	ENABLE_KYRA = STATIC_PLUGIN
	BUILD=scummvm-F
endif

ifdef DS_BUILD_G
	DEFINES = -DDS_BUILD_G
	LOGO = logog.bmp
	ENABLE_LURE = STATIC_PLUGIN
	BUILD=scummvm-G
endif

ifdef DS_BUILD_H
	DEFINES = -DDS_BUILD_H
	LOGO = logoh.bmp
	ENABLE_PARALLACTION = STATIC_PLUGIN
	BUILD=scummvm-H
endif

ifdef DS_BUILD_I
	DEFINES = -DDS_BUILD_I
	LOGO = logoi.bmp
	ENABLE_MADE = STATIC_PLUGIN
	BUILD=scummvm-I
endif

ifdef DS_BUILD_K
	DEFINES = -DDS_BUILD_K
	LOGO = logok.bmp
	ENABLE_CRUISE = STATIC_PLUGIN
	BUILD=scummvm-K
endif


#ifdef DS_BUILD_L
#	DEFINES = -DDS_BUILD_L
#	LOGO = logog.bmp
#	ENABLE_DRASCULA = STATIC_PLUGIN
#	BUILD=scummvm-K
#endif

#ifdef DS_BUILD_M
#	DEFINES = -DDS_BUILD_M
#	LOGO = logog.bmp
#	ENABLE_TUCKER = STATIC_PLUGIN
#	BUILD=scummvm-K
#endif

ARM7BIN	:= -7 $(CURDIR)/../../arm7/arm7.bin

CC      = arm-eabi-gcc
CXX     = arm-eabi-g++
LD	= arm-eabi-g++

CFLAGS	=	-Wno-multichar -Wall\
		-mcpu=arm9tdmi -mtune=arm9tdmi -fomit-frame-pointer\
		-mthumb-interwork -DUSE_ARM_COSTUME_ASM=1 -DDISABLE_SID


# -ffast-math

ifdef USE_DEBUGGER
	DEFINES += -DUSE_DEBUGGER
	CFLAGS += -g
endif

ifdef USE_PROFILER
	CFLAGS += -mpoke-function-name -finstrument-functions -g
	DEFINES += -DUSE_PROFILER
endif

CXXFLAGS=	$(CFLAGS) -Wno-non-virtual-dtor -Wno-unknown-pragmas -Wno-reorder  \
		-fno-exceptions -fno-rtti -mthumb-interwork -ffunction-sections -fdata-sections -fno-strict-aliasing

#		-mthumb

#		-fno-gcse\
#		-fno-schedule-insns2

#		-fno-inline-functions-called-once\
#		-fno-gcse-lm\
#		-fno-rerun-cse-after-loop\
#		-fno-cse-follow-jumps -fno-cse-skip-blocks\


ASFLAGS = -mcpu=arm9tdmi -mthumb-interwork
DEFINES += -D__DS__ -DNDS -DARM9 -DNONSTANDARD_PORT -DDISABLE_TEXT_CONSOLE -DDISABLE_FANCY_THEMES -DVECTOR_RENDERER_FORMAT=1555 -DDISABLE_DOSBOX_OPL -DDISABLE_DEFAULT_SAVEFILEMANAGER -DARM
ifdef USE_MAD
	DEFINES += -DUSE_MAD
endif

DEFINES += -DREDUCE_MEMORY_USAGE -DDISABLE_DEBUGGER -DUSE_TEXT_CONSOLE_FOR_DEBUGGER -DDISABLE_MASS_ADD -DDISABLE_NES_APU
# for release builds:
#DEFINES +=  -DNDEBUG

LDFLAGS = -specs=ds_arm9.specs -mthumb-interwork -mno-fpu -Wl,-Map,map.txt -Wl,--gc-sections

ifdef WRAP_MALLOC
	LDFLAGS += -Wl,--wrap,malloc
	DEFINES += -DWRAP_MALLOC
endif

BACKEND := ds

INCLUDES= -I$(portdir)/$(BUILD) -I$(srcdir) -I$(srcdir)/engines \
			-I$(portdir)/data -I$(portdir)/../commoninclude \
			-I$(portdir)/source -I$(portdir)/source/mad \
			-I$(libndsdir)/include -include $(portdir)/source/portdefs.h


LIBS	= -lm -L$(libndsdir)/lib -L$(portdir)/lib -lnds9
ifdef USE_MAD
	LIBS += -lmad
endif
ifdef USE_DEBUGGER
	LIBS += -ldsdebugger -ldswifi9
endif

EXECUTABLE = scummvm.elf
MKDIR = mkdir -p
RM = rm -f
RM_REC = rm -rf
AR = arm-eabi-ar cru
RANLIB = arm-eabi-ranlib
OBJCOPY = arm-eabi-objcopy
AS = arm-eabi-as
HAVE_GCC3 = true

PORT_OBJS := \
	$(portdir)/source/blitters_arm.o \
	$(portdir)/source/cdaudio.o \
	$(portdir)/source/dsmain.o \
	$(portdir)/source/gbampsave.o \
	$(portdir)/source/scummhelp.o \
	$(portdir)/source/osystem_ds.o \
	$(portdir)/source/touchkeyboard.o \
	$(portdir)/source/zipreader.o \
	$(portdir)/source/dsoptions.o \
	$(portdir)/source/keys.o \
	$(portdir)/source/wordcompletion.o \
	$(portdir)/source/interrupt.o

ifdef USE_PROFILER
	PORT_OBJS += $(portdir)/source/profiler/cyg-profile.o
endif


DATA_OBJS := \
	$(portdir)/data/icons.o \
	$(portdir)/data/keyboard.o \
	$(portdir)/data/keyboard_pal.o \
	$(portdir)/data/default_font.o \
	$(portdir)/data/8x8font_tga.o

FAT_OBJS := \
	$(portdir)/source/fat/disc_io.o \
	$(portdir)/source/fat/gba_nds_fat.o \
	$(portdir)/source/fat/io_fcsr.o \
	$(portdir)/source/fat/io_m3cf.o \
	$(portdir)/source/fat/io_mpcf.o \
	$(portdir)/source/fat/io_sccf.o \
	$(portdir)/source/fat/io_m3sd.o \
	$(portdir)/source/fat/io_nmmc.o \
	$(portdir)/source/fat/io_scsd.o \
	$(portdir)/source/fat/io_scsd_asm.o \
	$(portdir)/source/fat/io_njsd.o \
	$(portdir)/source/fat/io_mmcf.o \
	$(portdir)/source/fat/io_sd_common.o \
	$(portdir)/source/fat/io_m3_common.o \
	$(portdir)/source/fat/io_dldi.o \
	$(portdir)/source/fat/m3sd.o


# Files in this list will be optimisied for speed, otherwise they will be optimised for space
OPTLIST := actor.cpp dsmain.cpp osystem_ds.cpp blitters.cpp mame.cpp rate.cpp isomap.cpp image.cpp gfx.cpp sprite.cpp actor_path.cpp actor_walk.cpp script.cpp
#OPTLIST :=

# Compiler options for files which should be optimised for speed
ifdef DS_BUILD_E
  # Another attempt to save some RAM in ITE
  OPT_SPEED := -O3 -mthumb
else
  OPT_SPEED := -O3
endif

# Compiler options for files which should be optimised for space
OPT_SIZE := -Os -mthumb


OBJS := $(DATA_OBJS) $(PORT_OBJS) $(FAT_OBJS)



MODULE_DIRS += .

ndsall:
	@[ -d $(BUILD) ] || mkdir -p $(BUILD)
	$(MAKE) -C ./$(BUILD) -f ../makefile scummvm.nds scummvm.ds.gba

include $(srcdir)/Makefile.common

semiclean:
	$(RM) $(portdir)/source/dsoptions.o $(portdir)/source/dsmain.o $(FAT_OBJS) $(DATA_OBJS) $(portdir)/source/wordcompletion.o $(portdir)/source/dsoptions.o

clean:
	$(RM) $(OBJS) $(EXECUTABLE)
	rm -fr $(BUILD)

dist : SCUMMVM.BIN plugins plugin_dist


#---------------------------------------------------------------------------------
# canned command sequence for binary data
#---------------------------------------------------------------------------------
define bin2o
	bin2s $< | $(AS) -mthumb -mthumb-interwork -o $(@)
	echo "extern const u8" `(echo $(<F) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`"_end[];" > `(echo $(<F) | tr . _)`.h
	echo "extern const u8" `(echo $(<F) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`"[];" >> `(echo $(<F) | tr . _)`.h
	echo "extern const u32" `(echo $(<F) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`_size";" >> `(echo $(<F) | tr . _)`.h
endef


##############
# Replacement rule for the one in makefile.common
##############
%.o: %.cpp
	$(MKDIR) $(*D)/$(DEPDIR)
	$(CXX) -Wp,-MMD,"$(*D)/$(DEPDIR)/$(*F).d",-MQ,"$@",-MP $(if $(findstring $(notdir $<), $(OPTLIST)), $(OPT_SPEED), $(OPT_SIZE)) $(CXXFLAGS) $(CPPFLAGS) -c $(<) -o $*.o


#---------------------------------------------------------------------------------

#---------------------------------------------------------------------------------
%.o	:	%.bin
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

#---------------------------------------------------------------------------------
%.o	:	%.raw
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

#---------------------------------------------------------------------------------
%.o	:	%.pal
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

#---------------------------------------------------------------------------------
%.nds: %.bin
	ndstool -c $@ -9 $< $(ARM7BIN) -b ../../$(LOGO) "$(@F);ScummVM $(VERSION);DS Port"

%.ds.gba: %.nds
	dsbuild $< -o $@ -l $(portdir)/ndsloader.bin
	padbin 16 $@

#---------------------------------------------------------------------------------
%.bin: %.elf
	$(OBJCOPY) -S -O binary $< $@

#%.o: %.s
#	$(MKDIR) $(*D)/$(DEPDIR)
#	$(CXX) -Wp,-MMD,"$(*D)/$(DEPDIR)/$(*F).d",-MQ,"$@",-MP $(CXXFLAGS) $(CPPFLAGS) -c $(<) -o $*.o
