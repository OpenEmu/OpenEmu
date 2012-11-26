AUTOMAKE_OPTIONS = subdir-objects
DEFS = -DLOCALEDIR=\"$(datadir)/locale\" @DEFS@ @MATH_OPTIMIZER_FLAGS@
DEFAULT_INCLUDES = -I$(top_builddir)/include -I$(top_srcdir)/intl -I$(top_builddir)/include/blip

noinst_LIBRARIES	=	libmdfnsound.a

libmdfnsound_a_SOURCES  =       Blip_Buffer.cpp Stereo_Buffer.cpp Fir_Resampler.cpp WAVRecord.cpp
