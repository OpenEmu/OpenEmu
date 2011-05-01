AUTOMAKE_OPTIONS = subdir-objects
DEFS = -DLOCALEDIR=\"$(datadir)/locale\" @DEFS@ @MATH_OPTIMIZER_FLAGS@
DEFAULT_INCLUDES = -I$(top_builddir)/include -I$(top_srcdir)/intl -I$(top_builddir)/include/blip -I$(top_srcdir)

noinst_LIBRARIES        =       libmdfnhwsound.a

ym2413_SOURCES          =       ym2413/emu2413.cpp
ym2612_SOURCES          =       ym2612/Ym2612_Emu.cpp
gb_apu_SOURCES          =       gb_apu/Gb_Apu.cpp gb_apu/Gb_Apu_State.cpp gb_apu/Gb_Oscs.cpp
sms_apu_SOURCES         =       sms_apu/Sms_Apu.cpp
pce_psg_SOURCES		=	pce_psg/pce_psg.cpp

libmdfnhwsound_a_SOURCES        = $(ym2413_SOURCES) $(ym2612_SOURCES) $(gb_apu_SOURCES) $(sms_apu_SOURCES) $(pce_psg_SOURCES)

