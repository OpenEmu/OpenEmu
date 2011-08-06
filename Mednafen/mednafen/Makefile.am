SUBDIRS	= trio
AUTOMAKE_OPTIONS = subdir-objects
DEFS = -DLOCALEDIR=\"$(datadir)/locale\" @DEFS@ @MATH_OPTIMIZER_FLAGS@
DEFAULT_INCLUDES = -I$(top_builddir)/include -I$(top_builddir)/include/blip -I$(top_srcdir)/intl -I$(top_srcdir)

bin_PROGRAMS	=	mednafen
mednafen_SOURCES 	= 	cputest.cpp debug.cpp error.cpp mempatcher.cpp settings.cpp endian.cpp mednafen.cpp file.cpp general.cpp md5.cpp memory.cpp netplay.cpp state.cpp movie.cpp player.cpp psf.cpp tests.cpp qtrecord.cpp cdplay.cpp okiadpcm.cpp math_ops.cpp FileWrapper.cpp
mednafen_LDADD 		= 	trio/libtrio.a drivers/libmdfnsdl.a sexyal/libsexyal.a @SDL_LIBS@
mednafen_DEPENDENCIES	=	trio/libtrio.a drivers/libmdfnsdl.a sexyal/libsexyal.a

SUBDIRS			+=	sexyal drivers

if WANT_DEBUGGER
mednafen_SOURCES	+=	dis6280.cpp dis6502.cpp
endif

SUBDIRS += demo
mednafen_LDADD		+=	demo/libdemo.a
mednafen_DEPENDENCIES	+=	demo/libdemo.a

if WANT_GB_EMU
SUBDIRS += gb
mednafen_LDADD          +=      gb/libgb.a
mednafen_DEPENDENCIES   +=      gb/libgb.a
endif

if WANT_GBA_EMU
SUBDIRS += gba
mednafen_LDADD 		+=	gba/libgba.a
mednafen_DEPENDENCIES	+=	gba/libgba.a
endif

if WANT_LYNX_EMU
SUBDIRS += lynx
mednafen_LDADD          +=      lynx/liblynx.a
mednafen_DEPENDENCIES   +=      lynx/liblynx.a
endif

if WANT_MD_EMU
SUBDIRS += md
mednafen_LDADD          +=      md/libmd.a
mednafen_DEPENDENCIES   +=      md/libmd.a
endif

if WANT_NES_EMU
SUBDIRS += nes
mednafen_LDADD          +=      nes/libnes.a
mednafen_DEPENDENCIES   +=      nes/libnes.a
endif

if WANT_NGP_EMU
SUBDIRS += ngp
mednafen_LDADD          +=      ngp/libngp.a
mednafen_DEPENDENCIES   +=      ngp/libngp.a
endif

if WANT_PCE_EMU
SUBDIRS += pce
mednafen_LDADD          +=      pce/libpce.a
mednafen_DEPENDENCIES   +=      pce/libpce.a
endif

if WANT_PCE_FAST_EMU
SUBDIRS += pce_fast
mednafen_LDADD          +=      pce_fast/libpce_fast.a
mednafen_DEPENDENCIES   +=      pce_fast/libpce_fast.a
endif

if WANT_PCFX_EMU
SUBDIRS += pcfx
mednafen_LDADD          +=      pcfx/libpcfx.a
mednafen_DEPENDENCIES   +=      pcfx/libpcfx.a
endif

if WANT_SMS_EMU
SUBDIRS += sms
mednafen_LDADD          +=      sms/libsms.a
mednafen_DEPENDENCIES   +=      sms/libsms.a
endif

if WANT_SNES_EMU
SUBDIRS += snes
mednafen_LDADD          +=      snes/libsnes.a
mednafen_DEPENDENCIES   +=      snes/libsnes.a
endif

if WANT_VB_EMU
SUBDIRS += vb
mednafen_LDADD		+=	vb/libvb.a
mednafen_DEPENDENCIES	+=	vb/libvb.a
endif

if WANT_WSWAN_EMU
SUBDIRS += wswan
mednafen_LDADD          +=      wswan/libwswan.a
mednafen_DEPENDENCIES   +=      wswan/libwswan.a
endif

if WANT_DEBUGGER
SUBDIRS += desa68
mednafen_LDADD		+=	desa68/libdesa68.a
mednafen_DEPENDENCIES	+=	desa68/libdesa68.a
endif

SUBDIRS	+= hw_cpu
mednafen_LDADD		+=	hw_cpu/libmdfnhwcpu.a
mednafen_DEPENDENCIES	+=	hw_cpu/libmdfnhwcpu.a

SUBDIRS += hw_misc
mednafen_LDADD          +=      hw_misc/libmdfnhwmisc.a
mednafen_DEPENDENCIES   +=      hw_misc/libmdfnhwmisc.a

SUBDIRS += hw_sound
mednafen_LDADD          +=      hw_sound/libmdfnhwsound.a
mednafen_DEPENDENCIES   +=      hw_sound/libmdfnhwsound.a

SUBDIRS += hw_video
mednafen_LDADD          +=      hw_video/libmdfnhwvideo.a
mednafen_DEPENDENCIES   +=      hw_video/libmdfnhwvideo.a

SUBDIRS += tremor mpcdec
mednafen_LDADD		+=	tremor/libvorbisidec.a mpcdec/libmpcdec.a
mednafen_DEPENDENCIES	+=	tremor/libvorbisidec.a mpcdec/libmpcdec.a
include cdrom/Makefile.am.inc


SUBDIRS			+=	sound
mednafen_LDADD		+=	sound/libmdfnsound.a
mednafen_DEPENDENCIES	+=	sound/libmdfnsound.a

include compress/Makefile.am.inc
include string/Makefile.am.inc
include video/Makefile.am.inc


mednafen_LDADD		+= 	@LIBINTL@ @LIBICONV@

