AUTOMAKE_OPTIONS = subdir-objects
DEFS = -DLOCALEDIR=\"$(datadir)/locale\" @DEFS@ @MATH_OPTIMIZER_FLAGS@
DEFAULT_INCLUDES = -I$(top_builddir)/include -I$(top_srcdir)/intl

noinst_LIBRARIES	=	libnes.a
mednafen_SOURCES 	= 	nes.cpp x6502.cpp cart.cpp fds.cpp ines.cpp input.cpp nsf.cpp nsfe.cpp unif.cpp vsuni.cpp sound.cpp filter.cpp fds-sound.cpp

if WANT_DEBUGGER
mednafen_SOURCES	+=	debug.cpp
endif

include ppu/Makefile.am.inc
include boards/Makefile.am.inc
include input/Makefile.am.inc
#include mappers/Makefile.am.inc

libnes_a_SOURCES	=	$(mednafen_SOURCES) ntsc/nes_ntsc.cpp
