AUTOMAKE_OPTIONS = subdir-objects
DEFS = -DLOCALEDIR=\"$(datadir)/locale\" @DEFS@ @MATH_OPTIMIZER_FLAGS@ -fno-strict-aliasing
DEFAULT_INCLUDES = -I$(top_builddir)/include -I$(top_srcdir)/intl -I$(top_srcdir)

noinst_LIBRARIES	=	libvb.a
libvb_a_SOURCES 	= 	vb.cpp timer.cpp input.cpp vip.cpp vsu.cpp

if WANT_DEBUGGER
libvb_a_SOURCES		+=	debug.cpp
endif
