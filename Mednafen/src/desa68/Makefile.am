#lib_LTLIBRARIES = libdesa68.la
#libdesa68_la_SOURCES = desa68.c desa68.h
#libdesa68_la_LDFLAGS =  -version-info 2:0:0 -release @VERSION@
#libdesa68_la_CFLAGS = -Wno-multichar $(AM_CFLAGS)
##EXTRA_DIST = desa68.vcproj

AUTOMAKE_OPTIONS = subdir-objects
DEFS = -DLOCALEDIR=\"$(datadir)/locale\" @DEFS@ @MATH_OPTIMIZER_FLAGS@
DEFAULT_INCLUDES = -I$(top_builddir)/include -I$(top_srcdir)/intl

noinst_LIBRARIES	=	libdesa68.a
libdesa68_a_SOURCES	=	desa68.c
