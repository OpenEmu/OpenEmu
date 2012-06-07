AUTOMAKE_OPTIONS = subdir-objects
DEFS = -DLOCALEDIR=\"$(datadir)/locale\" @DEFS@ @MATH_OPTIMIZER_FLAGS@
DEFAULT_INCLUDES = -I$(top_builddir)/include -I$(top_srcdir)/intl

noinst_LIBRARIES	=	libwswan.a

wswan_SOURCES 	= gfx.cpp main.cpp memory.cpp v30mz.cpp sound.cpp tcache.cpp interrupt.cpp eeprom.cpp rtc.cpp

if WANT_DEBUGGER
wswan_SOURCES	+= dis/dis_decode.cpp dis/dis_groups.cpp dis/resolve.cpp dis/syntax.cpp debug.cpp
endif

libwswan_a_SOURCES	=	$(wswan_SOURCES)
