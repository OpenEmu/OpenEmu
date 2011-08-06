AUTOMAKE_OPTIONS = subdir-objects
DEFS = -DLOCALEDIR=\"$(datadir)/locale\" @DEFS@ @MATH_OPTIMIZER_FLAGS@
DEFAULT_INCLUDES = -I$(top_builddir)/include -I$(top_srcdir)/intl

noinst_LIBRARIES	=	liblynx.a
liblynx_a_SOURCES 	= 	cart.cpp c65c02.cpp memmap.cpp mikie.cpp ram.cpp rom.cpp susie.cpp system.cpp
