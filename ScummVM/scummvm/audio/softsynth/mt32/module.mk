MODULE := audio/softsynth/mt32

MODULE_OBJS := \
	AReverbModel.o \
	DelayReverb.o \
	FreeverbModel.o \
	LA32Ramp.o \
	Part.o \
	Partial.o \
	PartialManager.o \
	Poly.o \
	Synth.o \
	TVA.o \
	TVF.o \
	TVP.o \
	Tables.o \
	freeverb.o

# Include common rules
include $(srcdir)/rules.mk
