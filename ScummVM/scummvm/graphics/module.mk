MODULE := graphics

MODULE_OBJS := \
	conversion.o \
	cursorman.o \
	font.o \
	fontman.o \
	fonts/bdf.o \
	fonts/consolefont.o \
	fonts/newfont_big.o \
	fonts/newfont.o \
	fonts/ttf.o \
	fonts/winfont.o \
	iff.o \
	maccursor.o \
	primitives.o \
	scaler.o \
	scaler/thumbnail_intern.o \
	sjis.o \
	surface.o \
	thumbnail.o \
	VectorRenderer.o \
	VectorRendererSpec.o \
	wincursor.o \
	yuv_to_rgb.o \
	decoders/bmp.o \
	decoders/jpeg.o \
	decoders/pcx.o \
	decoders/pict.o \
	decoders/png.o \
	decoders/tga.o

ifdef USE_SCALERS
MODULE_OBJS += \
	scaler/2xsai.o \
	scaler/aspect.o \
	scaler/downscaler.o \
	scaler/scale2x.o \
	scaler/scale3x.o \
	scaler/scalebit.o

ifdef USE_ARM_SCALER_ASM
MODULE_OBJS += \
	scaler/downscalerARM.o \
	scaler/scale2xARM.o \
	scaler/Normal2xARM.o
endif

ifdef USE_HQ_SCALERS
MODULE_OBJS += \
	scaler/hq2x.o \
	scaler/hq3x.o

ifdef USE_NASM
MODULE_OBJS += \
	scaler/hq2x_i386.o \
	scaler/hq3x_i386.o
endif

endif

endif

# Include common rules
include $(srcdir)/rules.mk
