AUTOMAKE_OPTIONS = subdir-objects
DEFS = -DLOCALEDIR=\"$(datadir)/locale\" @DEFS@ @MATH_OPTIMIZER_FLAGS@
DEFAULT_INCLUDES = -I$(top_builddir)/include -I$(top_srcdir)/intl -I$(top_srcdir)/src/hw_cpu -I$(top_srcdir)/src/hw_sound


noinst_LIBRARIES	=	libmd.a

libmd_a_SOURCES         =	vdp.cpp genesis.cpp genio.cpp header.cpp mem68k.cpp membnk.cpp memvdp.cpp memz80.cpp sound.cpp system.cpp

libmd_a_SOURCES 	+= 	cart/cart.cpp cart/map_eeprom.cpp cart/map_realtec.cpp cart/map_ssf2.cpp cart/map_ff.cpp cart/map_rom.cpp cart/map_sbb.cpp cart/map_yase.cpp cart/map_rmx3.cpp cart/map_sram.cpp

libmd_a_SOURCES		+=	input/mouse.cpp input/gamepad.cpp

libmd_a_SOURCES		+=	cd/cd.cpp cd/timer.cpp cd/interrupt.cpp cd/pcm.cpp cd/cdc_cdd.cpp

if WANT_DEBUGGER
libmd_a_SOURCES		+=	debug.cpp
endif
