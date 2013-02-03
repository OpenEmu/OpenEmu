#/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
# *   Mupen64plus-video-glide64 - Makefile                                  *
# *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
# *   Copyright (C) 2010 Jon Ring                                           *
# *   Copyright (C) 2007-2009 Richard Goedeken                              *
# *   Copyright (C) 2007-2008 DarkJeztr Tillin9                             *
# *                                                                         *
# *   This program is free software; you can redistribute it and/or modify  *
# *   it under the terms of the GNU General Public License as published by  *
# *   the Free Software Foundation; either version 2 of the License, or     *
# *   (at your option) any later version.                                   *
# *                                                                         *
# *   This program is distributed in the hope that it will be useful,       *
# *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
# *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
# *   GNU General Public License for more details.                          *
# *                                                                         *
# *   You should have received a copy of the GNU General Public License     *
# *   along with this program; if not, write to the                         *
# *   Free Software Foundation, Inc.,                                       *
# *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
# * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
# Makefile for Glide64mk2 plugin in Mupen64Plus

# detect operating system
UNAME ?= $(shell uname -s)
OS := NONE
ifeq ("$(UNAME)","Linux")
  OS = LINUX
  SO_EXTENSION = so
  SHARED = -shared
endif
ifeq ("$(UNAME)","linux")
  OS = LINUX
  SO_EXTENSION = so
  SHARED = -shared
endif
ifneq ("$(filter GNU hurd,$(UNAME))","")
  OS = LINUX
  SO_EXTENSION = so
  SHARED = -shared
endif
ifeq ("$(UNAME)","Darwin")
  OS = OSX
  SO_EXTENSION = dylib
  SHARED = -bundle
endif
ifeq ("$(UNAME)","FreeBSD")
  OS = FREEBSD
  SO_EXTENSION = so
  SHARED = -shared
endif
ifeq ("$(UNAME)","OpenBSD")
  OS = FREEBSD
  SO_EXTENSION = so
  SHARED = -shared
  $(warning OS type "$(UNAME)" not officially supported.')
endif
ifneq ("$(filter GNU/kFreeBSD kfreebsd,$(UNAME))","")
  OS = LINUX
  SO_EXTENSION = so
  SHARED = -shared
endif
ifeq ("$(patsubst MINGW%,MINGW,$(UNAME))","MINGW")
  OS = MINGW
  SO_EXTENSION = dll
  SHARED = -shared
  PIC = 0
  BOOST_SUFFIX = -mt
  BOOST_THREAD_SUFFIX = _win32-mt
endif
ifeq ("$(OS)","NONE")
  $(error OS type "$(UNAME)" not supported.  Please file bug report at 'http://code.google.com/p/mupen64plus/issues')
endif

# detect system architecture
HOST_CPU ?= $(shell uname -m)
CPU := NONE
ifneq ("$(filter x86_64 amd64,$(HOST_CPU))","")
  CPU := X86
  ifeq ("$(BITS)", "32")
    ARCH_DETECTED := 64BITS_32
    PIC ?= 0
  else
    ARCH_DETECTED := 64BITS
    PIC ?= 1
  endif
endif
ifneq ("$(filter pentium i%86,$(HOST_CPU))","")
  CPU := X86
  ARCH_DETECTED := 32BITS
  PIC ?= 0
endif
ifneq ("$(filter ppc powerpc,$(HOST_CPU))","")
  CPU := PPC
  ARCH_DETECTED := 32BITS
  BIG_ENDIAN := 1
  PIC ?= 1
  NO_ASM := 1
  $(warning Architecture "$(HOST_CPU)" not officially supported.')
endif
ifneq ("$(filter ppc64 powerpc64,$(HOST_CPU))","")
  CPU := PPC
  ARCH_DETECTED := 64BITS
  BIG_ENDIAN := 1
  PIC ?= 1
  NO_ASM := 1
  $(warning Architecture "$(HOST_CPU)" not officially supported.')
endif
ifneq ("$(filter arm%,$(HOST_CPU))","")
  ifeq ("$(filter arm%b,$(HOST_CPU))","")
    CPU := ARM
    ARCH_DETECTED := 32BITS
    PIC ?= 1
    NO_ASM := 1
    $(warning Architecture "$(HOST_CPU)" not officially supported.')
  endif
endif
ifeq ("$(CPU)","NONE")
  $(error CPU type "$(HOST_CPU)" not supported.  Please file bug report at 'http://code.google.com/p/mupen64plus/issues')
endif

# base CFLAGS, LDLIBS, and LDFLAGS
OPTFLAGS ?= -O3
WARNFLAGS ?= -Wall
CFLAGS += $(OPTFLAGS) $(WARNFLAGS) -ffast-math -fno-strict-aliasing -fvisibility=hidden -I../../src -I../../src/Glitch64/inc -DGCC
CXXFLAGS += -fvisibility-inlines-hidden
LDFLAGS += $(SHARED)
BOOST_SUFFIX ?= -mt
BOOST_THREAD_SUFFIX ?= -mt
LDLIBS += -lboost_filesystem$(BOOST_SUFFIX) -lboost_system$(BOOST_SUFFIX) -lboost_thread$(BOOST_THREAD_SUFFIX)

ifeq ($(OS), MINGW)
  CFLAGS += -DBOOST_THREAD_USE_LIB
  LDLIBS += -lpthread
else
  CFLAGS += -pthread
  LDLIBS += -pthread
endif

# default configuration programs
SDL_CONFIG = $(CROSS_COMPILE)sdl-config
PKG_CONFIG = $(CROSS_COMPILE)pkg-config

ifeq ($(CPU), X86)
  CFLAGS +=  -mmmx -msse
endif

# Since we are building a shared library, we must compile with -fPIC on some architectures
# On 32-bit x86 systems we do not want to use -fPIC because we don't have to and it has a big performance penalty on this arch
ifeq ($(PIC), 1)
  CFLAGS += -fPIC
  LDFLAGS += -fPIC
else
  CFLAGS += -fno-PIC
  LDFLAGS += -fno-PIC
endif

ifeq ($(BIG_ENDIAN), 1)
  CFLAGS += -DM64P_BIG_ENDIAN
endif

# tweak flags for 32-bit build on 64-bit system
ifeq ($(ARCH_DETECTED), 64BITS_32)
  ifeq ($(OS), FREEBSD)
    $(error Do not use the BITS=32 option with FreeBSD, use -m32 and -m elf_i386)
  endif
  CFLAGS += -m32
  LDFLAGS += -m32 -Wl,-m,elf_i386
endif

# set special flags per-system
ifeq ($(OS), LINUX)
  # only export api symbols
  LDFLAGS += -Wl,-version-script,$(SRCDIR)/video_api_export.ver
  LDLIBS += -ldl
endif
ifeq ($(OS), OSX)
  # Select the proper SDK
  # Also, SDKs are stored in a different location since XCode 4.3
  OSX_SDK ?= $(shell sw_vers -productVersion | cut -f1 -f2 -d .)
  OSX_XCODEMAJ = $(shell xcodebuild -version | grep '[0-9]*\.[0-9]*' | cut -f2 -d ' ' | cut -f1 -d .)
  OSX_XCODEMIN = $(shell xcodebuild -version | grep '[0-9]*\.[0-9]*' | cut -f2 -d ' ' | cut -f2 -d .)
  OSX_XCODEGE43 = $(shell echo "`expr $(OSX_XCODEMAJ) \>= 4``expr $(OSX_XCODEMIN) \>= 3`")
  ifeq ($(OSX_XCODEGE43), 11)
    OSX_SYSROOT := /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs
  else
    OSX_SYSROOT := /Developer/SDKs
  endif

  ifeq ($(CPU), X86)
    ifeq ($(ARCH_DETECTED), 64BITS)
      CFLAGS += -pipe -arch x86_64 -mmacosx-version-min=$(OSX_SDK) -isysroot $(OSX_SYSROOT)/MacOSX$(OSX_SDK).sdk
      LDFLAGS += -bundle -arch x86_64
      LDLIBS += -ldl
    else
      CFLAGS += -pipe -mmmx -msse -fomit-frame-pointer -arch i686 -mmacosx-version-min=$(OSX_SDK) -isysroot $(OSX_SYSROOT)/MacOSX$(OSX_SDK).sdk
      LDFLAGS += -bundle -arch i686
      LDLIBS += -ldl
    endif
  endif
endif
ifeq ($(OS), FREEBSD)
  LDLIBS += -lc
endif

# search for OpenGL libraries
GL_LIBS=
ifeq ($(OS), OSX)
  GL_LIBS = -framework OpenGL
endif
ifeq ($(OS), MINGW)
  GL_LIBS = -lopengl32
endif

ifeq ($(shell which $(PKG_CONFIG) 2>/dev/null),)
  $(error $(PKG_CONFIG) not found)
endif
ifeq ($(shell pkg-config --modversion libpng 2>/dev/null),)
  $(error No libpng development libraries found!)
endif
ifeq ($(OS), OSX) # use system zlib on OSX
  LDLIBS += -lz
else
  ifeq ($(shell $(PKG_CONFIG) --modversion zlib 2>/dev/null),)
    $(error No zlib development libraries found!)
  endif
  CFLAGS += $(shell $(PKG_CONFIG) --cflags zlib)
  LDLIBS +=  $(shell $(PKG_CONFIG) --libs zlib)
endif

CFLAGS += $(shell $(PKG_CONFIG) --cflags libpng)
LDLIBS +=  $(shell $(PKG_CONFIG) --libs libpng)

ifeq ("$(GL_LIBS)", "")
  ifeq ($(shell $(PKG_CONFIG) --modversion gl 2>/dev/null),)
    $(error No OpenGL development libraries found!)
  endif
  CFLAGS += $(shell $(PKG_CONFIG) --cflags gl)
  GL_LIBS = $(shell $(PKG_CONFIG) --libs gl)
endif
LDLIBS += $(GL_LIBS)

# test for presence of SDL
ifeq ($(shell which $(SDL_CONFIG) 2>/dev/null),)
  $(error No SDL development libraries found!)
endif
ifeq ($(OS),FREEBSD)
    CFLAGS  += $(shell $(SDL_CONFIG) --cflags) -D__unix__
    LDLIBS += $(shell $(SDL_CONFIG) --libs)
endif
ifeq ($(OS),OSX)
    CFLAGS  += $(shell $(SDL_CONFIG) --cflags) -D__unix__
    # sdl-config on mac screws up when we're trying to build a library and not an executable
    # SDL 1.3 is supposed to fix that, if it's ever released
    LDLIBS += -L/usr/local/lib -lSDL -Wl,-framework,Cocoa
endif
ifeq ($(OS),LINUX)
    CFLAGS  += $(shell $(SDL_CONFIG) --cflags) -D__unix__
    LDLIBS += $(shell $(SDL_CONFIG) --libs)
endif
ifeq ($(OS), MINGW)
  CFLAGS += $(shell $(SDL_CONFIG) --cflags)
  LDLIBS += $(shell $(SDL_CONFIG) --libs)
endif

# set mupen64plus core API header path
ifneq ("$(APIDIR)","")
  CFLAGS += "-I$(APIDIR)"
else
  TRYDIR = ../../../mupen64plus-core/src/api
  ifneq ("$(wildcard $(TRYDIR)/m64p_types.h)","")
    CFLAGS += -I$(TRYDIR)
  else
    TRYDIR = /usr/local/include/mupen64plus
    ifneq ("$(wildcard $(TRYDIR)/m64p_types.h)","")
      CFLAGS += -I$(TRYDIR)
    else
      TRYDIR = /usr/include/mupen64plus
      ifneq ("$(wildcard $(TRYDIR)/m64p_types.h)","")
        CFLAGS += -I$(TRYDIR)
      else
        $(error Mupen64Plus API header files not found! Use makefile parameter APIDIR to force a location.)
      endif
    endif
  endif
endif

# reduced compile output when running make without V=1
ifneq ($(findstring $(MAKEFLAGS),s),s)
ifndef V
	Q_CC  = @echo '    CC  '$@;
	Q_CXX = @echo '    CXX '$@;
	Q_LD  = @echo '    LD  '$@;
endif
endif

# set base program pointers and flags
CC        = $(CROSS_COMPILE)gcc
CXX       = $(CROSS_COMPILE)g++
RM       ?= rm -f
INSTALL  ?= install
MKDIR ?= mkdir -p
COMPILE.c = $(Q_CC)$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c
COMPILE.cc = $(Q_CXX)$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c
LINK.o = $(Q_LD)$(CXX) $(LDFLAGS) $(TARGET_ARCH)

# set special flags for given Makefile parameters
ifeq ($(DEBUG),1)
  CFLAGS += -g
  INSTALL_STRIP_FLAG ?= 
else
  INSTALL_STRIP_FLAG ?= -s
endif
ifeq ($(NO_ASM), 1)
  CFLAGS += -DNO_ASM
endif

# set installation options
ifeq ($(PREFIX),)
  PREFIX := /usr/local
endif
ifeq ($(SHAREDIR),)
  SHAREDIR := $(PREFIX)/share/mupen64plus
endif
ifeq ($(LIBDIR),)
  LIBDIR := $(PREFIX)/lib
endif
ifeq ($(PLUGINDIR),)
  PLUGINDIR := $(LIBDIR)/mupen64plus
endif

SRCDIR = ../../src
OBJDIR = _obj$(POSTFIX)

# list of source files to compile
SOURCE = \
	$(SRCDIR)/Glitch64/combiner.cpp \
	$(SRCDIR)/Glitch64/geometry.cpp \
	$(SRCDIR)/Glitch64/main.cpp \
	$(SRCDIR)/Glitch64/textures.cpp \
	$(SRCDIR)/Glide64/3dmath.cpp \
	$(SRCDIR)/Glide64/Combine.cpp \
	$(SRCDIR)/Glide64/Config.cpp \
	$(SRCDIR)/Glide64/CRC.cpp \
	$(SRCDIR)/Glide64/Debugger.cpp \
	$(SRCDIR)/Glide64/DepthBufferRender.cpp \
	$(SRCDIR)/Glide64/FBtoScreen.cpp \
	$(SRCDIR)/Glide64/Ini.cpp \
	$(SRCDIR)/Glide64/Keys.cpp \
	$(SRCDIR)/Glide64/Main.cpp \
	$(SRCDIR)/Glide64/rdp.cpp \
	$(SRCDIR)/Glide64/TexBuffer.cpp \
	$(SRCDIR)/Glide64/TexCache.cpp \
	$(SRCDIR)/Glide64/Util.cpp \
	$(SRCDIR)/GlideHQ/Ext_TxFilter.cpp \
	$(SRCDIR)/GlideHQ/TxFilterExport.cpp \
	$(SRCDIR)/GlideHQ/TxFilter.cpp \
	$(SRCDIR)/GlideHQ/TxCache.cpp \
	$(SRCDIR)/GlideHQ/TxTexCache.cpp \
	$(SRCDIR)/GlideHQ/TxHiResCache.cpp \
	$(SRCDIR)/GlideHQ/TxQuantize.cpp \
	$(SRCDIR)/GlideHQ/TxUtil.cpp \
	$(SRCDIR)/GlideHQ/TextureFilters.cpp \
	$(SRCDIR)/GlideHQ/TextureFilters_2xsai.cpp \
	$(SRCDIR)/GlideHQ/TextureFilters_hq2x.cpp \
	$(SRCDIR)/GlideHQ/TextureFilters_hq4x.cpp \
	$(SRCDIR)/GlideHQ/TxImage.cpp \
	$(SRCDIR)/GlideHQ/TxReSample.cpp \
	$(SRCDIR)/GlideHQ/TxDbg.cpp \
	$(SRCDIR)/GlideHQ/tc-1.1+/fxt1.c \
	$(SRCDIR)/GlideHQ/tc-1.1+/dxtn.c \
	$(SRCDIR)/GlideHQ/tc-1.1+/wrapper.c \
	$(SRCDIR)/GlideHQ/tc-1.1+/texstore.c

ifeq ($(OS),MINGW)
SOURCE += $(SRCDIR)/Glide64/osal_dynamiclib_win32.c
else
SOURCE += $(SRCDIR)/Glide64/osal_dynamiclib_unix.c
endif



# generate a list of object files build, make a temporary directory for them
OBJECTS := $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(filter %.c, $(SOURCE)))
OBJECTS += $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(filter %.cpp, $(SOURCE)))
OBJDIRS = $(dir $(OBJECTS))
$(shell $(MKDIR) $(OBJDIRS))

# development only support for using asm on i386 linux
ifeq ($(OLD_ASM),1)

SOURCE = $(SRCDIR)/Glide64/Texture.asm
#CFLAGS += -DOLDASM_asmTextureCRC
#CFLAGS += -DOLDASM_asmLoad4bCI
#CFLAGS += -DOLDASM_asmLoad4bIAPal
#CFLAGS += -DOLDASM_asmLoad4bIA
#CFLAGS += -DOLDASM_asmLoad4bI
#CFLAGS += -DOLDASM_asmLoad16bRGBA
#CFLAGS += -DOLDASM_asmLoad16bIA
#CFLAGS += -DOLDASM_asmLoad8bCI
#CFLAGS += -DOLDASM_asmLoad8bIA8
#CFLAGS += -DOLDASM_asmLoad8bIA4
#CFLAGS += -DOLDASM_asmLoad8bI
#CFLAGS += -DOLDASM_asmMirror8bS
#CFLAGS += -DOLDASM_asmWrap8bS
#CFLAGS += -DOLDASM_asmClamp8bS
#CFLAGS += -DOLDASM_asmMirror16bS
#CFLAGS += -DOLDASM_asmWrap16bS
#CFLAGS += -DOLDASM_asmClamp16bS
#CFLAGS += -DOLDASM_asmMirror32bS
#CFLAGS += -DOLDASM_asmWrap32bS
#CFLAGS += -DOLDASM_asmClamp32bS
#CFLAGS += -DOLDASM_asmTexConv_ARGB1555_ARGB4444
#CFLAGS += -DOLDASM_asmTexConv_AI88_ARGB4444
#CFLAGS += -DOLDASM_asmTexConv_AI44_ARGB4444
#CFLAGS += -DOLDASM_asmTexConv_A8_ARGB4444
#CFLAGS += -DOLDASM_asmLoadBlock
#CFLAGS += -DOLDASM_asmLoadTile

OBJECTS += $(patsubst $(SRCDIR)/%.asm, $(OBJDIR)/%.o, $(filter %.asm, $(SOURCE)))

AS          = nasm
COMPILE.asm = $(AS) -O6 -felf -I$(SRCDIR)/Glide64/

$(OBJDIR)/%.o: $(SRCDIR)/%.asm
	$(COMPILE.asm) -o $@ $<

endif

# build targets
TARGET = mupen64plus-video-glide64mk2$(POSTFIX).$(SO_EXTENSION)

targets:
	@echo "Mupen64plus-video-glide64mk2 N64 Graphics plugin makefile. "
	@echo "  Targets:"
	@echo "    all           == Build Mupen64plus-video-glide64mk2 plugin"
	@echo "    clean         == remove object files"
	@echo "    rebuild       == clean and re-build all"
	@echo "    install       == Install Mupen64Plus-video-glide64mk2 plugin"
	@echo "    uninstall     == Uninstall Mupen64Plus-video-glide64mk2 plugin"
	@echo "  Options:"
	@echo "    BITS=32       == build 32-bit binaries on 64-bit machine"
	@echo "    NO_ASM=1      == build without inline assembly code (x86 MMX/SSE)"
	@echo "    APIDIR=path   == path to find Mupen64Plus Core headers"
	@echo "    OPTFLAGS=flag == compiler optimization (default: -O3)"
	@echo "    WARNFLAGS=flag == compiler warning levels (default: -Wall)"
	@echo "    PIC=(1|0)     == Force enable/disable of position independent code"
	@echo "    POSTFIX=name  == String added to the name of the the build (default: '')"
	@echo "  Install Options:"
	@echo "    PREFIX=path   == install/uninstall prefix (default: /usr/local)"
	@echo "    SHAREDIR=path == path to install shared data files (default: PREFIX/share/mupen64plus)"
	@echo "    LIBDIR=path   == library prefix (default: PREFIX/lib)"
	@echo "    PLUGINDIR=path == path to install plugin libraries (default: LIBDIR/mupen64plus)"
	@echo "    DESTDIR=path  == path to prepend to all installation paths (only for packagers)"
	@echo "  Debugging Options:"
	@echo "    DEBUG=1       == add debugging symbols"
	@echo "    LTO=1         == enable experimental build with link-time optimization"
	@echo "    V=1           == show verbose compiler output"

all: $(TARGET)

install: $(TARGET)
	$(INSTALL) -d "$(DESTDIR)$(PLUGINDIR)"
	$(INSTALL) -m 0644 $(INSTALL_STRIP_FLAG) $(TARGET) "$(DESTDIR)$(PLUGINDIR)"
	$(INSTALL) -d "$(DESTDIR)$(SHAREDIR)"
	$(INSTALL) -m 0644 "../../data/Glide64mk2.ini" "$(DESTDIR)$(SHAREDIR)"

uninstall:
	$(RM) "$(DESTDIR)$(PLUGINDIR)/$(TARGET)"
	$(RM) "$(DESTDIR)$(SHAREDIR)/Glide64mk2.ini"

clean:
	$(RM) -r $(OBJDIR) $(TARGET)

rebuild: clean all

# build dependency files
CFLAGS += -MD
-include $(OBJECTS:.o=.d)

CXXFLAGS += $(CFLAGS)
ifeq ($(LTO), 1)
  CFLAGS += -flto -ffunction-sections -fdata-sections
  CXXFLAGS += -flto -ffunction-sections -fdata-sections
  LDFLAGS += -fuse-linker-plugin -Wl,--gc-sections $(CXXFLAGS)
endif

# standard build rules
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(COMPILE.c) -o $@ $<

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(COMPILE.cc) -o $@ $<

$(TARGET): $(OBJECTS)
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

.PHONY: all clean install uninstall targets
