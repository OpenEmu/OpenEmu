AUTOMAKE_OPTIONS = subdir-objects
DEFS = -DLOCALEDIR=\"$(datadir)/locale\" @DEFS@ @MATH_OPTIMIZER_FLAGS@
DEFAULT_INCLUDES = -I$(top_builddir)/include -I$(top_srcdir)/intl

noinst_LIBRARIES	=	libpcfx.a
libpcfx_a_SOURCES 	= 	v810_cpu.cpp v810_cpuD.cpp soundbox.cpp pcfx.cpp king.cpp vdc.cpp interrupt.cpp input.cpp timer.cpp rainbow.cpp jrevdct.cpp huc6273.cpp

if WANT_DEBUGGER
libpcfx_a_SOURCES	+=	debug.cpp
endif
