# ScummVM Makefile for Windows CE port
# Uses the cegcc toolchain. For build info check out the wiki: http://wiki.scummvm.org

########################################################################
## Do you want a debug build or not?

#WINCE_DEBUG_BUILD = 1
#UNOPTIMIZED_BUILD = 1

########################################################################
## Do you want a build using plugins?

#DYNAMIC_MODULES = 1
# TODO: You'll need to change STATIC_PLUGIN to DYNAMIC_PLUGIN below

########################################################################
## Enable whichever engines you want here

ENABLE_SCUMM = STATIC_PLUGIN
ENABLE_SCUMM_7_8 = 1
ENABLE_HE = 1
ENABLE_SKY = STATIC_PLUGIN
ENABLE_QUEEN = STATIC_PLUGIN
ENABLE_GOB = STATIC_PLUGIN
ENABLE_LURE = STATIC_PLUGIN
ENABLE_CINE = STATIC_PLUGIN
ENABLE_SAGA = STATIC_PLUGIN
ENABLE_IHNM = 1
#ENABLE_SAGA2 = 1
ENABLE_KYRA = STATIC_PLUGIN
ENABLE_AGI = STATIC_PLUGIN
ENABLE_AGOS = STATIC_PLUGIN
ENABLE_SWORD1 = STATIC_PLUGIN
ENABLE_SWORD2 = STATIC_PLUGIN
ENABLE_TOUCHE = STATIC_PLUGIN
ENABLE_PARALLACTION = STATIC_PLUGIN
ENABLE_DRASCULA = STATIC_PLUGIN
ENABLE_GROOVIE = STATIC_PLUGIN
ENABLE_TUCKER = STATIC_PLUGIN
ENABLE_TINSEL = STATIC_PLUGIN
ENABLE_CRUISE = STATIC_PLUGIN
ENABLE_MADE = STATIC_PLUGIN
#ENABLE_SCI = STATIC_PLUGIN
#ENABLE_M4 = STATIC_PLUGIN

########################################################################
## Pick which libraries you want to use here

USE_MAD           = 1
#USE_TREMOR        = 1
USE_TREMOLO       = 1
#USE_FLAC          = 1
USE_ZLIB          = 1

########################################################################
## For remote deployment (copying the built files onto a device
## automatically), we need to know various things, like which
## tools to use. I use pput and pdel from
##     http://www.xs4all.nl/~itsme/projects/xda/tools.html
## and I keep my installation on the storage card, but other people
## may prefer other things.

REMOTE_COPY      = pput
REMOTE_DELETE    = pdel
REMOTE_DIRECTORY = \Storage Card\Program Files\Scummvm

########################################################################
## You're probably not going to want to change these defines...

USE_SCALERS = 1
USE_HQ_SCALERS = 1
USE_ARM_SOUND_ASM = 1
USE_ARM_SMUSH_ASM = 1
USE_ARM_GFX_ASM   = 1
USE_ARM_COSTUME_ASM = 1
USE_ARM_SCALER_ASM = 1

########################################################################
##    Hopefully you shouldn't need to change anything below here.     ##
########################################################################


srcdir    = ../../..
VPATH     = $(srcdir)

CXX    = arm-wince-mingw32ce-g++
LD     = arm-wince-mingw32ce-g++
AR     = arm-wince-mingw32ce-ar cru
RANLIB = arm-wince-mingw32ce-ranlib
STRIP  = arm-wince-mingw32ce-strip
WINDRES= arm-wince-mingw32ce-windres
MKDIR  = mkdir -p
RM     = rm -f
RM_REC = rm -rf
ECHO   = echo -n
CAT    = cat
AS     = arm-wince-mingw32ce-as

########################################################################
## Set up defines, includes, cflags etc

DEFINES := -D_WIN32_WCE=300 -D__ARM__ -D_ARM_ -DUNICODE -DSCUMM_NEED_ALIGNMENT
DEFINES += -DFPM_DEFAULT

DEFINES += -DNONSTANDARD_PORT
DEFINES += -DWIN32
DEFINES += -Dcdecl= -D__cdecl__= -Wno-multichar

INCLUDES := -I$(srcdir) -I. -I$(srcdir)/engines -Imissing/gcc -Ilibs/include -Ilibs/include/sdl -ICEgui -ICEkeys

CFLAGS :=
ifndef UNOPTIMIZED_BUILD
CFLAGS += -O3 -fno-inline-functions -march=armv4 -mtune=xscale
endif

LDFLAGS := -Wl,-Map,scummvm.exe.map -Wl,--stack,65536
LIBS := -Llibs/lib -lSDL

ifdef WINCE_DEBUG_BUILD
DEFINES += -DDEBUG -DUSE_WINDBG
CFLAGS += -g
LDFLAGS += -debug
endif

ifdef USE_ZLIB
DEFINES += -DUSE_ZLIB
LIBS += -lzlib
endif

ifdef USE_MAD
DEFINES += -DUSE_MAD
LIBS += -lmad
endif

ifdef USE_TREMOR
DEFINES += -DUSE_TREMOR -DUSE_VORBIS
LIBS += -ltremorce
endif

ifdef USE_TREMOLO
DEFINES += -DUSE_TREMOR -DUSE_VORBIS -DUSE_TREMOLO
INCLUDES += -Ilibs/include/tremolo
LIBS += -llibTremolo
endif

ifdef USE_FLAC
DEFINES += -DUSE_FLAC
LIBS += -lFLAC
endif

ifdef USE_SCALERS
DEFINES += -DUSE_SCALERS
endif

ifdef USE_HQ_SCALERS
DEFINES += -DUSE_HQ_SCALERS
endif

ifdef USE_ARM_SMUSH_ASM
DEFINES += -DUSE_ARM_SMUSH_ASM
endif

ifdef USE_ARM_GFX_ASM
DEFINES += -DUSE_ARM_GFX_ASM
endif

ifdef USE_ARM_COSTUME_ASM
DEFINES += -DUSE_ARM_COSTUME_ASM
endif

ifdef USE_ARM_SCALER_ASM
DEFINES += -DUSE_ARM_SCALER_ASM
endif

########################################################################
# Targets follow here

EXECUTABLE = scummvm.exe
CXXFLAGS := $(CFLAGS)
OBJS :=
MODULE_DIRS += ./
DEPDIR = .deps

OBJS += CEActionsPocket.o CEDevice.o CEScaler.o \
        CEActionsSmartphone.o CELauncherDialog.o wince-sdl.o
OBJS += CEgui/GUIElement.o CEgui/Panel.o CEgui/SDL_ImageResource.o \
        CEgui/ItemAction.o CEgui/PanelItem.o CEgui/Toolbar.o \
        CEgui/ItemSwitch.o CEgui/PanelKeyboard.o CEgui/ToolbarHandler.o
OBJS += CEkeys/EventsBuffer.o
OBJS += $(srcdir)/gui/Actions.o $(srcdir)/gui/Key.o $(srcdir)/gui/KeysDialog.o
OBJS += ../sdl/sdl.o ../sdl/graphics.o ../sdl/events.o ../sdl/hardwarekeys.o
OBJS += missing/missing.o
OBJS += smartLandScale.o
ifndef DYNAMIC_MODULES
OBJS += PocketSCUMM.o
endif

ifdef DYNAMIC_MODULES
EXECUTABLE		 = scummvm.dll
PLUGIN_PREFIX		:=
PLUGIN_SUFFIX		:= .dll
PLUGIN_EXTRA_DEPS	 = $(EXECUTABLE)
CXXFLAGS		+= -DDYNAMIC_MODULES
PLUGIN_LDFLAGS		:= -shared -L. -lscummvm -Llibs/lib
PRE_OBJS_FLAGS		:= -Wl,--whole-archive
POST_OBJS_FLAGS		:= -Wl,--export-all-symbols -Wl,--no-whole-archive -Wl,--out-implib,./libscummvm.a -shared
endif

include $(srcdir)/Makefile.common

########################################################################
## Our top level target, and what we deploy depends on whether we are
## building the plugin version or not...
ifdef DYNAMIC_MODULES

all: dist-plugins

deploy: deploy-plugins deploy-exe

else

all: dist

deploy: deploy-exe

endif

dist: PocketSCUMM.o $(EXECUTABLE)

dist-plugins: $(EXECUTABLE) plugins PocketSCUMM.o plugins-stub

plugins-stub: stub.o
	$(CXX) stub.o PocketSCUMM.o -L. -lscummvm -o scummvm.exe

PocketSCUMM.o: PocketSCUMM.rc
	$(WINDRES) $*.rc $@

all-clean: clean
	$(RM) PocketSCUMM.o stub.o

deploy-plugins: dist-plugins
	@echo Deploying DLLs
	-$(REMOTE_MKDIR) "$(REMOTE_DIRECTORY)"
	-$(REMOTE_DELETE) "$(REMOTE_DIRECTORY)\*.dll"
	$(REMOTE_COPY) plugins/*.dll "$(REMOTE_DIRECTORY)"
	$(REMOTE_COPY) scummvm.dll "$(REMOTE_DIRECTORY)"

deploy-exe: dist
	@echo Deploying modern theme - errors are normal here!
	-$(REMOTE_MKDIR) "$(REMOTE_DIRECTORY)"
	@-$(REMOTE_COPY) ../../../gui/themes/modern.ini "$(REMOTE_DIRECTORY)"
	@-$(REMOTE_COPY) ../../../gui/themes/modern.zip "$(REMOTE_DIRECTORY)"
	@echo Deploying EXE
	-$(REMOTE_DELETE) "$(REMOTE_DIRECTORY)\scummvm.exe"
	$(REMOTE_COPY) scummvm.exe "$(REMOTE_DIRECTORY)"
