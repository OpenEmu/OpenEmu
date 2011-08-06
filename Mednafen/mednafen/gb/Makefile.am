AUTOMAKE_OPTIONS = subdir-objects
DEFS = -DLOCALEDIR=\"$(datadir)/locale\" @DEFS@ @MATH_OPTIMIZER_FLAGS@ 
DEFAULT_INCLUDES = -I$(top_builddir)/include -I$(top_srcdir)/intl -I$(top_builddir)/include/blip -I$(top_srcdir)/src/hw_sound

noinst_LIBRARIES        =       libgb.a
gb_SOURCES     =	gb.cpp gfx.cpp gbGlobals.cpp memory.cpp sound.cpp z80.cpp

libgb_a_SOURCES        =       $(gb_SOURCES)
