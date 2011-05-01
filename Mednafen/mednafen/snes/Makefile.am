AUTOMAKE_OPTIONS = subdir-objects
DEFS = -DLOCALEDIR=\"$(datadir)/locale\" @DEFS@ @MATH_OPTIMIZER_FLAGS@ -DNOMINMAX
AM_CXXFLAGS = -Wno-shadow -Wno-unused -Wno-sign-compare -Wno-inline
# -Wno-c++0x-compat
DEFAULT_INCLUDES = -I$(top_builddir)/include -I$(top_srcdir)/intl -I$(top_srcdir)/src/snes/src/lib

noinst_LIBRARIES	=	libsnes.a

meow			=	interface.cpp


meow	+=	src/lib/libco/libco.c


#meow	+=	src/lib/nall/string.cpp

#################
### utilities ###
#################

meow	+=	src/cartridge/cartridge.cpp src/cheat/cheat.cpp

##############
### memory ###
##############

meow	+=	src/memory/memory.cpp src/memory/smemory/smemory.cpp

###########
### cpu ###
###########

meow	+=	src/cpu/cpu.cpp src/cpu/core/core.cpp src/cpu/scpu/scpu.cpp

###########
### smp ###
###########

meow	+=	src/smp/smp.cpp src/smp/core/core.cpp src/smp/ssmp/ssmp.cpp

###########
### dsp ###
###########

# src/dsp/adsp/adsp.cpp
meow	+=	src/dsp/sdsp/sdsp.cpp

###########
### ppu ###
###########

meow	+=	src/ppu/ppu.cpp src/ppu/bppu/bppu.cpp

##############
### system ###
##############

meow	+=	src/system/system.cpp

#####################
### special chips ###
#####################

meow	+=	src/chip/sa1/sa1.cpp src/chip/bsx/bsx.cpp src/chip/srtc/srtc.cpp src/chip/sdd1/sdd1.cpp
meow	+=	src/chip/spc7110/spc7110.cpp src/chip/cx4/cx4.cpp

meow	+=	src/chip/dsp1/dsp1.cpp src/chip/dsp2/dsp2.cpp src/chip/dsp3/dsp3.cpp src/chip/dsp4/dsp4.cpp

meow	+=	src/chip/obc1/obc1.cpp

meow	+=	src/chip/st010/st010.cpp src/chip/st011/st011.cpp src/chip/st018/st018.cpp

meow	+=	src/chip/superfx/superfx.cpp

meow	+=	src/chip/supergameboy/supergameboy.cpp

meow	+=	src/chip/21fx/21fx.cpp

libsnes_a_SOURCES	=	$(meow)
