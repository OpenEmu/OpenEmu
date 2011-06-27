AUTOMAKE_OPTIONS = subdir-objects
DEFS = -DLOCALEDIR=\"$(datadir)/locale\" @DEFS@ @MATH_OPTIMIZER_FLAGS@
DEFAULT_INCLUDES = -I$(top_builddir)/include -I$(top_srcdir)/intl -I$(top_srcdir)/src/hw_cpu -I$(top_srcdir)/src/hw_sound

noinst_LIBRARIES        =       libsms.a

sms_SOURCES   = cart.cpp memz80.cpp  pio.cpp  render.cpp  romdb.cpp sms.cpp sound.cpp system.cpp tms.cpp vdp.cpp 

libsms_a_SOURCES      =       $(sms_SOURCES)

