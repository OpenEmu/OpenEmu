#/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
# *   Mupen64plus - Makefile                                                *
# *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
# *   Copyright (C) 2008-2009 Richard Goedeken                              *
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
# Makefile for Mupen64Plus Core

# detect operating system
UNAME = $(shell uname -s)
OS := NONE
ifeq ("$(UNAME)","Linux")
  OS = LINUX
endif
ifeq ("$(UNAME)","linux")
  OS = LINUX
endif
ifeq ("$(UNAME)","Darwin")
  OS = OSX
endif
ifeq ("$(UNAME)","FreeBSD")
  OS = FREEBSD
endif
ifneq ("$(filter GNU/kFreeBSD kfreebsd,$(UNAME))","")
  OS = LINUX
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
  else
    ARCH_DETECTED := 64BITS
  endif
endif
ifneq ("$(filter pentium i%86,$(HOST_CPU))","")
  CPU := X86
  ARCH_DETECTED := 32BITS
endif
# PPC doesn't work yet
#ifneq ("$(filter ppc powerpc,$(HOST_CPU))","")
#  CPU := PPC
#  ARCH_DETECTED := 32BITS
#  NO_ASM := 1
#endif
#ifneq ("$(filter ppc64 powerpc64,$(HOST_CPU))","")
#  CPU := PPC
#  ARCH_DETECTED := 64BITS
#  NO_ASM := 1
#endif
ifeq ("$(CPU)","NONE")
  $(error CPU type "$(HOST_CPU)" not supported.  Please file bug report at 'http://code.google.com/p/mupen64plus/issues')
endif

# base CFLAGS, LIBS, and LDFLAGS
CFLAGS += -Wall -ffast-math -funroll-loops -fexpensive-optimizations -fno-strict-aliasing -fvisibility=hidden -I../../src
CXXFLAGS += -fvisibility-inlines-hidden
LIBS += -ldl -lpng -lz -lm -lfreetype

# Since we are building a shared library, we must compile with -fPIC for x86_64 CPUs.
# On 32-bit systems we do not want to use -fPIC because we don't have to and it has a big performance penalty on this arch
ifeq ($(ARCH_DETECTED), 64BITS)
  CFLAGS += -fPIC -DPIC
endif
# tweak flags for 32-bit build on 64-bit system
ifeq ($(ARCH_DETECTED), 64BITS_32)
  ifeq ($(OS), FREEBSD)
    $(error Do not use the BITS=32 option with FreeBSD, use -m32 and -m elf_i386)
  endif
  CFLAGS += -m32
  LDFLAGS += -m32 -m elf_i386
endif

# set special flags per-system
ifeq ($(OS), FREEBSD)
  TARGET = libmupen64plus.so.2.0.0
  SONAME = libmupen64plus.so.2
  LDFLAGS += -Wl,-Bsymbolic -shared -Wl,-export-dynamic -Wl,-soname,$(SONAME)
  LIBS += -L${LOCALBASE}/lib -lGL -lGLU
endif
ifeq ($(OS), LINUX)
  TARGET = libmupen64plus.so.2.0.0
  SONAME = libmupen64plus.so.2
  LDFLAGS += -Wl,-Bsymbolic -shared -Wl,-export-dynamic -Wl,-soname,$(SONAME)
  LIBS += -L/usr/X11R6/lib -lGL -lGLU
  # only export api symbols
  LDFLAGS += -Wl,-version-script,$(SRCDIR)/api/api_export.ver
  ifeq ($(CPU), X86)
    ifeq ($(ARCH_DETECTED), 64BITS)
      CFLAGS += -pipe -O3 -march=athlon64
    else
      CFLAGS += -pipe -O3 -mmmx -msse -march=i686 -mtune=pentium-m
      ifneq ($(PROFILE), 1)
        CFLAGS += -fomit-frame-pointer
      endif
    endif
  endif
endif
ifeq ($(OS), OSX)
  TARGET = libmupen64plus.dylib
  LDFLAGS += -bundle -read_only_relocs suppress
  LIBS += -framework OpenGL
  ifeq ($(CPU), X86)
    ifeq ($(ARCH_DETECTED), 64BITS)
      CFLAGS += -pipe -O3 -arch x86_64 -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk
      LDFLAGS += -arch x86_64
    else
      CFLAGS += -pipe -O3 -mmmx -msse -arch i686 -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk
      LDFLAGS += -arch i686
      ifneq ($(PROFILE), 1)
        CFLAGS += -fomit-frame-pointer
      endif
    endif
  endif
endif
ifeq ($(CPU), PPC)
  CFLAGS += -mcpu=powerpc
endif
ifeq ($(CPU_ENDIANNESS), BIG)
  CFLAGS += -D_BIG_ENDIAN
endif

# test for presence of SDL
ifeq ($(shell which sdl-config 2>/dev/null),)
  $(error No SDL development libraries found!)
endif
ifeq ($(OS),FREEBSD)
    CFLAGS	+= `${SDL_CONFIG} --cflags`
    LIBS	+= `${SDL_CONFIG} --libs`
else
    CFLAGS	+= $(shell sdl-config --cflags)
    LIBS	+= $(shell sdl-config --libs)
endif

# test for presence of FreeType
ifeq ($(shell which freetype-config 2>/dev/null),)
   $(error freetype-config not installed!)
endif
LIBS	+= $(shell freetype-config --libs)
CFLAGS	+= $(shell freetype-config --cflags)

# set base program pointers and flags
CC       ?= gcc
CXX      ?= g++
RM       ?= rm
INSTALL  ?= install
LDCONFIG ?= ldconfig
ifeq ($(OS),OSX)
  STRIP	?= strip -x 
else
  STRIP	?= strip -s
endif

# compiler/linker flags for various compile-time options.
# 1. macro for no assembly language
ifeq ($(NO_ASM), 1)
  CFLAGS += -DNO_ASM
endif
# 2. variables for profiling and adding debugging symbols
ifeq ($(PROFILE), 1)
  CFLAGS += -pg -g
  LDFLAGS += -pg
  STRIP = true
else
  ifeq ($(DEBUG), 1)
    CFLAGS += -g
    STRIP = true
  endif
endif
# 3. other options given to the makefile on the command line
ifeq ($(LIRC), 1)
  CFLAGS += -DWITH_LIRC
endif
ifeq ($(DEBUGGER), 1)
  CFLAGS += -DDBG
endif
ifeq ($(DBG_COMPARE), 1)
  CFLAGS += -DCOMPARE_CORE
endif
ifeq ($(DBG_CORE), 1)
  CFLAGS += -DCORE_DBG
endif
ifeq ($(DBG_COUNT), 1)
  CFLAGS += -DCOUNT_INSTR
endif
ifeq ($(DBG_PROFILE), 1)
  CFLAGS += -DPROFILE_R4300
endif
# 4. compile-time directory paths for building into the library
ifneq ($(SHAREDIR),)
  CFLAGS += -DSHAREDIR="$(SHAREDIR)"
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
ifeq ($(INCDIR),)
  INCDIR := $(PREFIX)/include/mupen64plus
endif

SRCDIR = ../../src
OBJDIR = _obj

# list of required source files for compilation
SOURCE = \
	$(SRCDIR)/api/callbacks.c \
	$(SRCDIR)/api/common.c \
	$(SRCDIR)/api/config.c \
	$(SRCDIR)/api/debugger.c \
	$(SRCDIR)/api/frontend.c \
	$(SRCDIR)/api/vidext.c \
	$(SRCDIR)/main/main.c \
	$(SRCDIR)/main/util.c \
	$(SRCDIR)/main/cheat.c \
	$(SRCDIR)/main/eventloop.c \
	$(SRCDIR)/main/md5.c \
	$(SRCDIR)/main/rom.c \
	$(SRCDIR)/main/ini_reader.c \
	$(SRCDIR)/main/savestates.c \
	$(SRCDIR)/main/adler32.c \
	$(SRCDIR)/main/zip/ioapi.c \
	$(SRCDIR)/main/zip/zip.c \
	$(SRCDIR)/main/zip/unzip.c \
	$(SRCDIR)/memory/dma.c \
	$(SRCDIR)/memory/flashram.c \
	$(SRCDIR)/memory/memory.c \
	$(SRCDIR)/memory/pif.c \
	$(SRCDIR)/memory/tlb.c \
	$(SRCDIR)/osal/dynamiclib_unix.c \
	$(SRCDIR)/osal/files_unix.c \
	$(SRCDIR)/plugin/plugin.c \
	$(SRCDIR)/plugin/dummy_video.c \
	$(SRCDIR)/plugin/dummy_audio.c \
	$(SRCDIR)/plugin/dummy_input.c \
	$(SRCDIR)/plugin/dummy_rsp.c \
	$(SRCDIR)/r4300/r4300.c \
	$(SRCDIR)/r4300/bc.c \
	$(SRCDIR)/r4300/cop0.c \
	$(SRCDIR)/r4300/cop1.c \
	$(SRCDIR)/r4300/cop1_d.c \
	$(SRCDIR)/r4300/cop1_l.c \
	$(SRCDIR)/r4300/cop1_s.c \
	$(SRCDIR)/r4300/cop1_w.c \
	$(SRCDIR)/r4300/exception.c \
	$(SRCDIR)/r4300/interupt.c \
	$(SRCDIR)/r4300/profile.c \
	$(SRCDIR)/r4300/pure_interp.c \
	$(SRCDIR)/r4300/recomp.c \
	$(SRCDIR)/r4300/special.c \
	$(SRCDIR)/r4300/regimm.c \
	$(SRCDIR)/r4300/tlb.c \
	$(SRCDIR)/osd/OGLFT.cpp \
	$(SRCDIR)/osd/osd.cpp \
	$(SRCDIR)/osd/screenshot.cpp

# source files for optional features
ifneq ($(NO_ASM), 1)
  ifeq ($(CPU), X86)
    ifeq ($(ARCH_DETECTED), 64BITS)
      DYNAREC = x86_64
    else
      DYNAREC = x86
    endif
  endif
endif
ifneq ($(DYNAREC), )
  CFLAGS += -DDYNAREC
  SOURCE += \
    $(SRCDIR)/r4300/$(DYNAREC)/assemble.c \
    $(SRCDIR)/r4300/$(DYNAREC)/gbc.c \
    $(SRCDIR)/r4300/$(DYNAREC)/gcop0.c \
    $(SRCDIR)/r4300/$(DYNAREC)/gcop1.c \
    $(SRCDIR)/r4300/$(DYNAREC)/gcop1_d.c \
    $(SRCDIR)/r4300/$(DYNAREC)/gcop1_l.c \
    $(SRCDIR)/r4300/$(DYNAREC)/gcop1_s.c \
    $(SRCDIR)/r4300/$(DYNAREC)/gcop1_w.c \
    $(SRCDIR)/r4300/$(DYNAREC)/gr4300.c \
    $(SRCDIR)/r4300/$(DYNAREC)/gregimm.c \
    $(SRCDIR)/r4300/$(DYNAREC)/gspecial.c \
    $(SRCDIR)/r4300/$(DYNAREC)/gtlb.c \
    $(SRCDIR)/r4300/$(DYNAREC)/regcache.c \
    $(SRCDIR)/r4300/$(DYNAREC)/rjump.c
else
  SOURCE += $(SRCDIR)/r4300/empty_dynarec.c
endif

ifeq ($(LIRC), 1)
  SOURCE += $(SRCDIR)/main/lirc.c
  LIBS += -llirc_client
endif

ifeq ($(DEBUGGER), 1)
  SOURCE += \
	$(SRCDIR)/debugger/debugger.c \
	$(SRCDIR)/debugger/dbg_decoder.c \
	$(SRCDIR)/debugger/dbg_memory.c \
	$(SRCDIR)/debugger/dbg_breakpoints.c
  LIBS += -lopcodes -lbfd
endif

# generate a list of object files to build, make a temporary directory for them
OBJECTS := $(patsubst $(SRCDIR)/%.c,   $(OBJDIR)/%.o, $(filter %.c,   $(SOURCE)))
OBJECTS += $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(filter %.cpp, $(SOURCE)))
OBJDIRS = $(dir $(OBJECTS))
$(shell mkdir -p $(OBJDIRS))

# build targets
targets:
	@echo "Mupen64Plus-core makefile. "
	@echo "  Targets:"
	@echo "    all           == Build Mupen64Plus core library"
	@echo "    clean         == remove object files"
	@echo "    install       == Install Mupen64Plus core library"
	@echo "    uninstall     == Uninstall Mupen64Plus core library"
	@echo "  Build Options:"
	@echo "    BITS=32       == build 32-bit binaries on 64-bit machine"
	@echo "    LIRC=1        == enable LIRC support"
	@echo "    NO_ASM=1      == build without assembly (no dynamic recompiler or MMX/SSE code)"
	@echo "    SHAREDIR=path == extra path to search for shared data files"
	@echo "  Install Options:"
	@echo "    PREFIX=path   == install/uninstall prefix (default: /usr/local/)"
	@echo "    SHAREDIR=path == path to install shared data files (default: PREFIX/share/mupen64plus)"
	@echo "    LIBDIR=path   == path to install core library (default: PREFIX/lib)"
	@echo "    INCDIR=path   == path to install core header files (default: PREFIX/include/mupen64plus)"
	@echo "    DESTDIR=path  == path to prepend to all installation paths (only for packagers)"
	@echo "  Debugging Options:"
	@echo "    PROFILE=1     == build gprof instrumentation into binaries for profiling"
	@echo "    DEBUG=1       == add debugging symbols to binaries"
	@echo "    DEBUGGER=1    == build graphical debugger"
	@echo "    DBG_CORE=1    == print debugging info in r4300 core"
	@echo "    DBG_COUNT=1   == print R4300 instruction count totals (64-bit dynarec only)"
	@echo "    DBG_COMPARE=1 == enable core-synchronized r4300 debugging"
	@echo "    DBG_PROFILE=1 == dump profiling data for r4300 dynarec to data file"

all: $(TARGET)

install: $(TARGET)
	$(INSTALL) -d -v "$(DESTDIR)$(LIBDIR)"
	$(INSTALL) -m 0644 $(TARGET) "$(DESTDIR)$(LIBDIR)"
	$(INSTALL) -d -v "$(DESTDIR)$(SHAREDIR)"
	$(INSTALL) -m 0644 ../../data/* "$(DESTDIR)$(SHAREDIR)"
	$(INSTALL) -d -v "$(DESTDIR)$(INCDIR)"
	$(INSTALL) -m 0644 ../../src/api/m64p_*.h "$(DESTDIR)$(INCDIR)"
	if [ `id -u` -eq 0 ]; then $(LDCONFIG); fi

uninstall:
	$(RM) -f "$(DESTDIR)$(LIBDIR)/$(TARGET)"
	if [ "$(SONAME)" != "" ]; then $(RM) -f "$(DESTDIR)$(LIBDIR)/$(SONAME)"; fi
	$(RM) -f $(DESTDIR)$(INCDIR)/m64p_*.h
	$(RM) -f "$(DESTDIR)$(SHAREDIR)/mupen64plus.cht"
	$(RM) -f "$(DESTDIR)$(SHAREDIR)/mupen64plus.ini"
	$(RM) -f "$(DESTDIR)$(SHAREDIR)/font.ttf"

clean:
	$(RM) -rf $(TARGET) $(SONAME) ./_obj

# build dependency files
CFLAGS += -MD
-include $(OBJECTS:.o=.d)

CXXFLAGS += $(CFLAGS)

# reduced compile output when running make without V=1
ifneq ($(findstring $(MAKEFLAGS),s),s)
ifndef V
	Q_CC  = @echo '    CC  '$@;
	Q_CXX = @echo '    CXX '$@;
	Q_LD  = @echo '    LD  '$@;
endif
endif

# build rules
$(TARGET): $(OBJECTS)
	$(Q_LD)$(CXX) $(LDFLAGS) $^ $(LIBS) -o $@
	$(STRIP) $@
	if [ "$(SONAME)" != "" ]; then ln -sf $@ $(SONAME); fi

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(Q_CXX)$(CXX) -o $@ $(CXXFLAGS) -c $<

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(Q_CC)$(CC) -o $@ $(CFLAGS) -c $<
