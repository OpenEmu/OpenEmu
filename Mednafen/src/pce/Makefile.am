AUTOMAKE_OPTIONS = subdir-objects
DEFS = -DLOCALEDIR=\"$(datadir)/locale\" @DEFS@ @MATH_OPTIMIZER_FLAGS@
DEFAULT_INCLUDES = -I$(top_builddir)/include -I$(top_srcdir)/intl

noinst_LIBRARIES	=	libpce.a
mednafen_SOURCES 	= 	huc6280.cpp pce.cpp vdc.cpp input.cpp psg.cpp huc.cpp hes.cpp adpcm.cpp cdrom.cpp tsushin.cpp

if WANT_DEBUGGER
mednafen_SOURCES	+=	debug.cpp
endif

libpce_a_SOURCES	=	$(mednafen_SOURCES)
