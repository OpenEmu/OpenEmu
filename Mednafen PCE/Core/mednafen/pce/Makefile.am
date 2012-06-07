AUTOMAKE_OPTIONS = subdir-objects
DEFS = -DLOCALEDIR=\"$(datadir)/locale\" @DEFS@ @MATH_OPTIMIZER_FLAGS@
DEFAULT_INCLUDES = -I$(top_builddir)/include -I$(top_srcdir)/intl -I$(top_srcdir) -I$(top_srcdir)/src/hw_cpu -I$(top_srcdir)/src/hw_misc -I$(top_srcdir)/src/hw_sound -I$(top_srcdir)/src/hw_video

noinst_LIBRARIES	=	libpce.a
mednafen_SOURCES 	= 	vce.cpp pce.cpp input.cpp huc.cpp hes.cpp tsushin.cpp subhw.cpp mcgenjin.cpp
mednafen_SOURCES	+=	input/gamepad.cpp input/tsushinkb.cpp input/mouse.cpp
if WANT_DEBUGGER
mednafen_SOURCES	+=	debug.cpp
endif

libpce_a_SOURCES	=	$(mednafen_SOURCES)
