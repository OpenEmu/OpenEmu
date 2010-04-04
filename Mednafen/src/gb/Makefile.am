AUTOMAKE_OPTIONS = subdir-objects
DEFS = -DLOCALEDIR=\"$(datadir)/locale\" @DEFS@ @MATH_OPTIMIZER_FLAGS@
DEFAULT_INCLUDES = -I$(top_builddir)/include -I$(top_srcdir)/intl

noinst_LIBRARIES        =       libgb.a
gb_SOURCES     =	gb.cpp gfx.cpp gbGlobals.cpp memory.cpp sound.cpp sound/Gb_Apu.cpp sound/Gb_Oscs.cpp z80.cpp

libgb_a_SOURCES        =       $(gb_SOURCES)
