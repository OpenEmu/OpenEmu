AUTOMAKE_OPTIONS = subdir-objects
DEFS = -DLOCALEDIR=\"$(datadir)/locale\" @DEFS@ @MATH_OPTIMIZER_FLAGS@
DEFAULT_INCLUDES = -I$(top_builddir)/include -I$(top_srcdir)/intl -I$(top_srcdir) -I$(top_srcdir)/src/hw_cpu -I$(top_srcdir)/src/hw_sound -I$(top_srcdir)/src/hw_video

noinst_LIBRARIES	=	libpcfx.a
libpcfx_a_SOURCES 	= 	king.cpp soundbox.cpp pcfx.cpp interrupt.cpp input.cpp timer.cpp rainbow.cpp jrevdct.cpp huc6273.cpp fxscsi.cpp

libpcfx_a_SOURCES	+=	input/gamepad.cpp input/mouse.cpp

if WANT_DEBUGGER
libpcfx_a_SOURCES	+=	debug.cpp
endif
