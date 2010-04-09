SUBDIRS	= trio
AUTOMAKE_OPTIONS = subdir-objects
DEFS = -DLOCALEDIR=\"$(datadir)/locale\" @DEFS@ @MATH_OPTIMIZER_FLAGS@
DEFAULT_INCLUDES = -I$(top_builddir)/include -I$(top_srcdir)/intl

bin_PROGRAMS	=	mednafen
mednafen_SOURCES 	= 	debug.cpp error.cpp mempatcher.cpp settings.cpp Blip_Buffer.cpp Stereo_Buffer.cpp endian.cpp mednafen.cpp file.cpp general.cpp md5.cpp memory.cpp netplay.cpp state.cpp movie.cpp wave.cpp player.cpp psf.cpp tests.cpp
mednafen_LDADD = @LIBINTL@ @LIBICONV@ trio/libtrio.a
mednafen_DEPENDENCIES	=	trio/libtrio.a

if WANT_DEBUGGER
mednafen_SOURCES	+=	dis6280.cpp dis6502.cpp
endif

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

if WANT_WSWAN_EMU
SUBDIRS += wswan
mednafen_LDADD          +=      wswan/libwswan.a
mednafen_DEPENDENCIES   +=      wswan/libwswan.a
endif


if NEED_Z80EMU
mednafen_SOURCES	+=	z80-fuse/z80.cpp z80-fuse/z80_ops.cpp
endif

if NEED_CDEMU
SUBDIRS += tremor mpcdec
mednafen_LDADD		+=	tremor/libvorbisidec.a mpcdec/libmpcdec.a
mednafen_DEPENDENCIES	+=	tremor/libvorbisidec.a mpcdec/libmpcdec.a
include cdrom/Makefile.am.inc
endif

if NEED_YM2612EMU
mednafen_SOURCES	+=	Ym2612_Emu.cpp
endif

include compress/Makefile.am.inc
include string/Makefile.am.inc
include video/Makefile.am.inc
include sexyal/Makefile.am.inc
include drivers/Makefile.am.inc

mednafen_SOURCES	+=	$(sexyal_SOURCES)

