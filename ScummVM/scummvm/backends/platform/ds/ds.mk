# To approximate the DS builds A, B, C, ... run our configure like this
#   configure --host=ds --disable-translation --disable-all-engines OPTIONS
# where OPTIONS is...
# build A:  --enable-scumm
# build B:  --enable-sky --enable-queen
# build C:  --enable-agos
# build D:  --enable-gob --enable-cine --enable-agi
# build E:  --enable-saga --disable-mad
# build F:  --enable-kyra --disable-mad
# build G:  --enable-lure
# build H:  --enable-parallaction
# build I:  --enable-made --disable-mad
# build K:  --enable-cruise --disable-mad
#
# However, this could be automated using a simple script, which generates
# subdirs for each build, and runs configure in those subdirs with the right
# parameters (all builds would still share the same set of source code files,
# thanks to our "out of tree" building support).
#
# This does not currently take care of some things:
# * It does not #define DS_BUILD_A etc. -- most uses of that should be
#   eliminated, though. Only usage should be for selecting the default config
#   file (and for that we should really rather allow overriding the value of
#   DEFAULT_CONFIG_FILE).
#   There are a few game specific hacks which are currently controlled by this,
#   too; we need to investigate those.
# * It does not currently adjust the logo. Ideally, if we ever get real plugin
#   support, that should be necessary anymore anyway.
# * No support for USE_DEBUGGER and USE_PROFILER yet. I envision that we would
#  integrate them with the --enable-debug and --enable-profiling configure options,
#  I simply haven't gotten around to do that yet.
# * ...

# Set location of ndsdir so that we can easily refer to files in it
ndsdir = backends/platform/ds


# Until we fix logo support, always use the A logo
LOGO = logoa.bmp

# Uncomment the following line to enable support for the
# ace DS Debugger (remembering to make the same change in the arm7 makefile):
#USE_DEBUGGER = 1
# TODO: Need to reimplement this (for arm9 and arm7).
#ifdef USE_DEBUGGER
#	DEFINES += -DUSE_DEBUGGER
#	CFLAGS += -g
#endif

# Uncomment the following line to enable the profiler
#USE_PROFILER = 1
# TODO: Need to reimplement this; and maybe replace it by the --enable-profiling
#       configure directive. Below is USE_PROFILER related code from the old NDS
#       build system:
#ifdef USE_PROFILER
#	CFLAGS += -mpoke-function-name -finstrument-functions -g
#	DEFINES += -DUSE_PROFILER
#endif
# And this for module.mk:
#ifdef USE_PROFILER
#	PORT_OBJS += arm9/source/profiler/cyg-profile.o
#endif



# NOTE: The header and libs for the debugger is assumed to be in the libnds
# folder.


ifdef WRAP_MALLOC
	LDFLAGS += -Wl,--wrap,malloc
	DEFINES += -DWRAP_MALLOC
endif



# Compiler options for files which should be optimised for speed
OPT_SPEED := -O3 -mno-thumb

# Compiler options for files which should be optimised for space
OPT_SIZE := -Os -mthumb

# By default optimize for size
CXXFLAGS += $(OPT_SIZE)

# Files listed below will be optimisied for speed, otherwise they will be optimised for space.
# TODO: Several of these files probably should not be optimized for speed, but for now
# we replicate the *precise* list from the old DS makefile, to ensure full compatibility.
# Eventually, we should tune this list.
$(ndsdir)/arm9/source/blitters.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
$(ndsdir)/arm9/source/dsmain.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
$(ndsdir)/arm9/source/osystem_ds.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
base/main.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
sound/rate.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
sound/softsynth/opl/mame.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/agi/sprite.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/agos/gfx.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/agos/script.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/cine/gfx.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/cruise/actor.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/cruise/script.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/draci/script.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/draci/sprite.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/gob/script.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/groovie/script.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/kyra/script.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/m4/actor.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/m4/script.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/m4/sprite.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/made/script.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/saga/actor_path.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/saga/actor_walk.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/saga/actor.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/saga/gfx.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/saga/image.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/saga/isomap.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/saga/script.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/saga/sprite.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/sci/engine/script.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/scumm/actor.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/scumm/gfx.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/scumm/script.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/sword2/sprite.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/teenagent/actor.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
# TODO: Fingolfin says: optimizing kyra/staticres.o for size would
# save about 30k, so maybe consider that?
#engines/kyra/staticres.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)



#############################################################################
#
# ARM9 rules.
#
#############################################################################

all: scummvm.nds scummvm.ds.gba

clean: dsclean

dsclean:
	$(RM) $(addprefix $(ndsdir)/, $(ARM7_MODULE_OBJS)) scummvm.nds scummvm.ds.gba

.PHONY: dsclean

# TODO: Add a 'dsdist' target ?

%.bin: %.elf
	$(OBJCOPY) -S -O binary $< $@

%.nds: %.bin $(ndsdir)/arm7/arm7.bin
	ndstool -c $@ -9 $< -7 $(ndsdir)/arm7/arm7.bin -b $(srcdir)/$(ndsdir)/$(LOGO) "$(@F);ScummVM $(VERSION);DS Port"

%.ds.gba: %.nds
	dsbuild $< -o $@ -l $(srcdir)/$(ndsdir)/arm9/ndsloader.bin
	padbin 16 $@

#############################################################################
#############################################################################
#############################################################################


#############################################################################
#
# ARM7 rules.
# For ARM7 files, we need different compiler flags, which leads to the
# extra rules for .o files below
#
#############################################################################

# HACK/FIXME: C compiler, for cartreset.c -- we should switch this to use CXX
# as soon as possible.
CC := $(DEVKITPRO)/devkitARM/bin/arm-eabi-gcc

# HACK/TODO: Pointer to objcopy. This should really be set by configure
OBJCOPY := $(DEVKITPRO)/devkitARM/bin/arm-eabi-objcopy

#
# Set various flags
#
ARM7_ARCH	:=	-mthumb-interwork

# note: arm7tdmi isn't the correct CPU arch, but anything newer and LD
# *insists* it has a FPU or VFP, and it won't take no for an answer!
ARM7_CFLAGS	:=	-g -Wall -O2\
		-mcpu=arm7tdmi -mtune=arm7tdmi -fomit-frame-pointer\
		-ffast-math \
		$(ARM7_ARCH) \
		-I$(srcdir)/$(ndsdir)/arm7/source/libcartreset \
		-I$(srcdir)/$(ndsdir)/commoninclude \
		-I$(DEVKITPRO)/libnds/include \
		-I$(DEVKITPRO)/libnds/include/nds \
		-DARM7

ARM7_CXXFLAGS	:= $(ARM7_CFLAGS) -fno-exceptions -fno-rtti

ARM7_LDFLAGS	:= -g $(ARM7_ARCH) -mno-fpu

# HACK/FIXME: Define a custom build rule for cartreset.c.
# We do this because it is a .c file, not a .cpp file and so is outside our
# regular build system anyway. But this is *bad*. It should be changed into a
# .cpp file and this rule be removed.
%.o: %.c
	$(MKDIR) $(*D)/$(DEPDIR)
	$(CC) -Wp,-MMD,"$(*D)/$(DEPDIR)/$(*F).d",-MQ,"$@",-MP $(CXXFLAGS) $(CPPFLAGS) -c $(<) -o $*.o

# Set custom build flags for cartreset.o
$(ndsdir)/arm7/source/libcartreset/cartreset.o: CXXFLAGS=$(ARM7_CFLAGS)
$(ndsdir)/arm7/source/libcartreset/cartreset.o: CPPFLAGS=

# Set custom build flags for main.o
$(ndsdir)/arm7/source/main.o: CXXFLAGS=$(ARM7_CXXFLAGS)
$(ndsdir)/arm7/source/main.o: CPPFLAGS=

# Rule for creating ARM7 .elf files by linking .o files together with a special linker script
$(ndsdir)/arm7/arm7.elf: \
	$(ndsdir)/arm7/source/libcartreset/cartreset.o \
	$(ndsdir)/arm7/source/main.o
	$(CXX) $(ARM7_LDFLAGS) -specs=ds_arm7.specs $+ -L$(DEVKITPRO)/libnds/lib -lnds7  -o $@

# Rule for creating ARM7 .bin files from .elf files
$(ndsdir)/arm7/arm7.bin: $(ndsdir)/arm7/arm7.elf
	$(OBJCOPY) -O binary  $< $@







# Command to build libmad is:
# ./configure --host=arm-elf --enable-speed --enable-sso -enable-fpm=arm CFLAGS='-specs=ds_arm9.specs -mthumb-interwork'
#
# I actually had to use
# ./configure --host=arm-elf --enable-speed --enable-sso -enable-fpm=arm CFLAGS='-specs=ds_arm9.specs -mthumb-interwork' LDFLAGS='C:/Progra~1/devkitpro/libnds/lib/libnds9.a' --disable-shared --disable-debugging
#
# Fingolfin used
# CXX=arm-eabi-g++ CC=arm-eabi-gcc ./configure --host=arm-elf --enable-speed --enable-sso -enable-fpm=arm CFLAGS='-specs=ds_arm9.specs -mthumb-interwork' --disable-shared --disable-debugging LDFLAGS=$DEVKITPRO/libnds/lib/libnds9.a
