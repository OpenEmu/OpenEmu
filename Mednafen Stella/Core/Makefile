TARGET := libretro-stella.dylib

MEDNAFEN_DIR := mednafen
STELLA_DIR := $(MEDNAFEN_DIR)/stella

STELLA_SOURCES := $(STELLA_DIR)/stella.cpp	\
	$(STELLA_DIR)/Console.cpp \
	$(STELLA_DIR)/Settings.cpp \
	$(STELLA_DIR)/SoundSDL.cpp \
	$(STELLA_DIR)/cart/Cart0840.cpp \
	$(STELLA_DIR)/cart/Cart2K.cpp \
	$(STELLA_DIR)/cart/Cart3E.cpp \
	$(STELLA_DIR)/cart/Cart3F.cpp \
	$(STELLA_DIR)/cart/Cart4A50.cpp \
	$(STELLA_DIR)/cart/Cart4K.cpp \
	$(STELLA_DIR)/cart/CartAR.cpp \
	$(STELLA_DIR)/cart/Cart.cpp	 \
	$(STELLA_DIR)/cart/CartCV.cpp \
	$(STELLA_DIR)/cart/CartDPC.cpp \
	$(STELLA_DIR)/cart/CartDPCPlus.cpp \
	$(STELLA_DIR)/cart/CartE0.cpp \
	$(STELLA_DIR)/cart/CartE7.cpp \
	$(STELLA_DIR)/cart/CartEF.cpp \
	$(STELLA_DIR)/cart/CartEFSC.cpp \
	$(STELLA_DIR)/cart/CartF0.cpp \
	$(STELLA_DIR)/cart/CartF4.cpp \
	$(STELLA_DIR)/cart/CartF4SC.cpp \
	$(STELLA_DIR)/cart/CartF6.cpp \
	$(STELLA_DIR)/cart/CartF6SC.cpp \
	$(STELLA_DIR)/cart/CartF8.cpp \
	$(STELLA_DIR)/cart/CartF8SC.cpp \
	$(STELLA_DIR)/cart/CartFA.cpp \
	$(STELLA_DIR)/cart/CartFE.cpp \
	$(STELLA_DIR)/cart/CartMC.cpp \
	$(STELLA_DIR)/cart/CartSB.cpp \
	$(STELLA_DIR)/cart/CartUA.cpp \
	$(STELLA_DIR)/cart/CartX07.cpp \
	$(STELLA_DIR)/cart/Thumbulator.cpp \
	$(STELLA_DIR)/input/AtariVox.cpp \
	$(STELLA_DIR)/input/Booster.cpp \
	$(STELLA_DIR)/input/Control.cpp \
	$(STELLA_DIR)/input/Driving.cpp \
	$(STELLA_DIR)/input/Genesis.cpp \
	$(STELLA_DIR)/input/Joystick.cpp \
	$(STELLA_DIR)/input/Keyboard.cpp \
	$(STELLA_DIR)/input/KidVid.cpp \
	$(STELLA_DIR)/input/MT24LC256.cpp \
	$(STELLA_DIR)/input/Paddles.cpp \
	$(STELLA_DIR)/input/SaveKey.cpp \
	$(STELLA_DIR)/input/Switches.cpp \
	$(STELLA_DIR)/input/TrackBall.cpp \
	$(STELLA_DIR)/system/M6502.cpp \
	$(STELLA_DIR)/system/M6532.cpp \
	$(STELLA_DIR)/system/NullDev.cpp \
	$(STELLA_DIR)/system/System.cpp \
	$(STELLA_DIR)/system/TIA.cpp \
	$(STELLA_DIR)/system/TIASnd.cpp \
	$(STELLA_DIR)/system/TIATables.cpp \
	$(STELLA_DIR)/utility/MD5.cpp \
	$(STELLA_DIR)/utility/Random.cpp \
	$(STELLA_DIR)/utility/Serializer.cpp \
	$(STELLA_DIR)/properties/Props.cpp \
	$(STELLA_DIR)/properties/PropsSet.cpp
	
	

MEDNAFEN_SOURCES := $(MEDNAFEN_DIR)/cdrom/cdromif.cpp \
	$(MEDNAFEN_DIR)/mednafen.cpp \
	$(MEDNAFEN_DIR)/PSFLoader.cpp \
	$(MEDNAFEN_DIR)/error.cpp \
	$(MEDNAFEN_DIR)/math_ops.cpp \
	$(MEDNAFEN_DIR)/settings.cpp \
	$(MEDNAFEN_DIR)/netplay.cpp \
	$(MEDNAFEN_DIR)/general.cpp \
	$(MEDNAFEN_DIR)/player.cpp \
	$(MEDNAFEN_DIR)/cdplay.cpp \
	$(MEDNAFEN_DIR)/FileWrapper.cpp \
	$(MEDNAFEN_DIR)/state.cpp \
	$(MEDNAFEN_DIR)/tests.cpp \
	$(MEDNAFEN_DIR)/movie.cpp \
	$(MEDNAFEN_DIR)/endian.cpp \
	$(MEDNAFEN_DIR)/qtrecord.cpp \
	$(MEDNAFEN_DIR)/cdrom/CDAccess.cpp \
	$(MEDNAFEN_DIR)/cdrom/CDAccess_Image.cpp \
	$(MEDNAFEN_DIR)/cdrom/CDUtility.cpp \
	$(MEDNAFEN_DIR)/cdrom/lec.cpp \
	$(MEDNAFEN_DIR)/cdrom/SimpleFIFO.cpp \
	$(MEDNAFEN_DIR)/cdrom/audioreader.cpp \
	$(MEDNAFEN_DIR)/cdrom/galois.cpp \
	$(MEDNAFEN_DIR)/cdrom/pcecd.cpp \
	$(MEDNAFEN_DIR)/cdrom/scsicd.cpp \
	$(MEDNAFEN_DIR)/cdrom/recover-raw.cpp \
	$(MEDNAFEN_DIR)/cdrom/l-ec.cpp \
	$(MEDNAFEN_DIR)/cdrom/crc32.cpp \
	$(MEDNAFEN_DIR)/memory.cpp \
	$(MEDNAFEN_DIR)/mempatcher.cpp \
	$(MEDNAFEN_DIR)/video/video.cpp \
	$(MEDNAFEN_DIR)/video/text.cpp \
	$(MEDNAFEN_DIR)/video/font-data.cpp \
	$(MEDNAFEN_DIR)/video/tblur.cpp \
	$(MEDNAFEN_DIR)/video/png.cpp \
	$(MEDNAFEN_DIR)/video/Deinterlacer.cpp \
	$(MEDNAFEN_DIR)/video/surface.cpp \
	$(MEDNAFEN_DIR)/video/resize.cpp \
	$(MEDNAFEN_DIR)/string/escape.cpp \
	$(MEDNAFEN_DIR)/string/ConvertUTF.cpp \
	$(MEDNAFEN_DIR)/sound/Blip_Buffer.cpp \
	$(MEDNAFEN_DIR)/sound/Fir_Resampler.cpp \
	$(MEDNAFEN_DIR)/sound/Stereo_Buffer.cpp \
	$(MEDNAFEN_DIR)/sound/WAVRecord.cpp \
	$(MEDNAFEN_DIR)/sound/sound.cpp \
	$(MEDNAFEN_DIR)/file.cpp \
	$(MEDNAFEN_DIR)/okiadpcm.cpp \
	$(MEDNAFEN_DIR)/md5.cpp

MPC_SRC := $(wildcard $(MEDNAFEN_DIR)/mpcdec/*.c)
TREMOR_SRC := $(wildcard $(MEDNAFEN_DIR)/tremor/*.c)

SOURCES_C := $(MEDNAFEN_DIR)/trio/trio.c \
	$(MPC_SRC) \
	$(TREMOR_SRC) \
	$(MEDNAFEN_DIR)/trio/trionan.c \
	$(MEDNAFEN_DIR)/trio/triostr.c \
	$(MEDNAFEN_DIR)/string/world_strtod.c \
	$(MEDNAFEN_DIR)/compress/blz.c \
	$(MEDNAFEN_DIR)/compress/unzip.c \
	$(MEDNAFEN_DIR)/compress/minilzo.c \
	$(MEDNAFEN_DIR)/compress/quicklz.c \
	$(MEDNAFEN_DIR)/compress/ioapi.c \
	$(MEDNAFEN_DIR)/resampler/resample.c

LIBRETRO_SOURCES := libretro.cpp stubs.cpp

SOURCES := $(LIBRETRO_SOURCES) $(HW_CPU_SOURCES) $(HW_MISC_SOURCES) $(HW_SOUND_SOURCES) $(HW_VIDEO_SOURCES) $(STELLA_SOURCES) $(MEDNAFEN_SOURCES)
OBJECTS := $(SOURCES:.cpp=.o) $(SOURCES_C:.c=.o)

all: $(TARGET)


LDFLAGS += -dynamiclib -fPIC -lz
FLAGS += -ffast-math -msse -msse2 -funroll-loops -O3 -g -Wall -fPIC -fno-strict-overflow
FLAGS += -I. -Imednafen -Imednafen/include -Imednafen/intl -Imednafen/hw_cpu -Imednafen/hw_misc -Imednafen/hw_sound -Imednafen/hw_video -pthread -Imednafen/stella -Imednafen/stella/cart -Imednafen/stella/input -Imednafen/stella/system -Imednafen/stella/utility -Imednafen/stella/properties

WARNINGS := -Wall \
	-Wno-narrowing \
	-Wno-unused-but-set-variable \
	-Wno-sign-compare \
	-Wno-unused-variable \
	-Wno-unused-function \
	-Wno-uninitialized \
	-Wno-unused-result \
	-Wno-strict-aliasing \
	-Wno-overflow

FLAGS += -DLSB_FIRST -DHAVE_MKDIR -DSIZEOF_DOUBLE=8 $(WARNINGS) \
			-DMEDNAFEN_VERSION=\"0.9.22\" -DMEDNAFEN_VERSION_NUMERIC=922 -DPSS_STYLE=1 -DMPC_FIXED_POINT -DARCH_X86 \
			-DWANT_STELLA_EMU -DSTDC_HEADERS

CXXFLAGS += $(FLAGS) -DHAVE_INTTYPES -DKeyboard=StellaKeyboard
CFLAGS += $(FLAGS) -std=gnu99

$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f $(TARGET) $(OBJECTS)

.PHONY: clean
