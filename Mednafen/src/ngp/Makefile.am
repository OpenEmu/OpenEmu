AUTOMAKE_OPTIONS = subdir-objects
DEFS = -DLOCALEDIR=\"$(datadir)/locale\" @DEFS@ @MATH_OPTIMIZER_FLAGS@ -fno-strict-aliasing
DEFAULT_INCLUDES = -I$(top_builddir)/include -I$(top_srcdir)/src/ngp -ITLCS-900h -I$(top_srcdir)/src/z80-fuse -I$(top_srcdir)/intl

noinst_LIBRARIES	=	libngp.a
ngp_SOURCES 	=	bios.cpp biosHLE.cpp dma.cpp flash.cpp gfx.cpp T6W28_Apu.cpp	\
	gfx_scanline_mono.cpp gfx_scanline_colour.cpp interrupt.cpp mem.cpp neopop.cpp \
	rom.cpp rtc.cpp sound.cpp Z80_interface.cpp \
	TLCS-900h/TLCS900h_disassemble_extra.cpp TLCS-900h/TLCS900h_disassemble_reg.cpp \
	TLCS-900h/TLCS900h_interpret_single.cpp TLCS-900h/TLCS900h_disassemble_src.cpp \
	TLCS-900h/TLCS900h_interpret.cpp TLCS-900h/TLCS900h_registers.cpp TLCS-900h/TLCS900h_interpret_reg.cpp \
	TLCS-900h/TLCS900h_disassemble.cpp TLCS-900h/TLCS900h_interpret_src.cpp \
	TLCS-900h/TLCS900h_interpret_dst.cpp TLCS-900h/TLCS900h_disassemble_dst.cpp

libngp_a_SOURCES	=	$(ngp_SOURCES)
