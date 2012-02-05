
# Makefile for genplus SDL
#
# (c) 1999, 2000, 2001, 2002, 2003  Charles MacDonald
# modified by Eke-Eke <eke_eke31@yahoo.fr>
#
# Defines :
# -DLSB_FIRST : for little endian systems.
# -DLOGERROR  : enable message logging
# -DLOGVDP    : enable VDP debug messages
# -DLOGSOUND  : enable AUDIO debug messages

NAME	  = gen_sdl.exe

CC        = gcc
CFLAGS    = `sdl-config --cflags` -march=i686 -O6 -fomit-frame-pointer -Wall -Wno-strict-aliasing
#-g -ggdb -pg
#-fomit-frame-pointer
#LDFLAGS   = -pg
DEFINES   = -DLSB_FIRST

INCLUDES  = -I. -I.. -I../z80 -I../m68k -I../sound -I../input_hw -I../cart_hw -I../cart_hw/svp -I../ntsc
LIBS	  = `sdl-config --libs` -lz -lm

OBJECTS	=       obj/z80.o	

OBJECTS	+=     	obj/m68kcpu.o	\
		obj/m68kops.o	


OBJECTS	+=     	obj/genesis.o	 \
		obj/vdp_ctrl.o	 \
		obj/vdp_render.o \
		obj/system.o     \
		obj/io_ctrl.o	 \
		obj/mem68k.o	 \
		obj/memz80.o	 \
		obj/membnk.o	 \
		obj/state.o     

OBJECTS	+=      obj/input.o	 \
		obj/gamepad.o	 \
		obj/lightgun.o	 \
		obj/mouse.o	 \
		obj/activator.o	 \
		obj/xe_a1p.o	 \
		obj/teamplayer.o \
		obj/paddle.o	 \
		obj/sportspad.o 


OBJECTS	+=      obj/sound.o	 \
		obj/sn76489.o    \
		obj/ym2612.o    

OBJECTS	+=	obj/Fir_Resampler.o
OBJECTS	+=	obj/blip.o 

OBJECTS	+=	obj/eq.o \

OBJECTS	+=      obj/sram.o      \
		obj/eeprom.o    \
		obj/svp.o	\
		obj/ssp16.o	\
		obj/ggenie.o	\
		obj/areplay.o	\
		obj/md_cart.o	\
		obj/sms_cart.o	

OBJECTS	+=	obj/main.o	\
		obj/config.o	\
		obj/error.o	\
		obj/unzip.o     \
		obj/fileio.o	\
		obj/loadrom.o	

OBJECTS	+=	obj/sms_ntsc.o	\
		obj/md_ntsc.o

OBJECTS	+=	obj/icon.o

all: $(NAME)

$(NAME): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $@

obj/%.o : 	../%.c ../%.h
		$(CC) -c $(CFLAGS) $(INCLUDES) $(DEFINES) $< -o $@
	        
obj/%.o :	../asm/%.s
		$(AS) $< -o $@
	        
obj/%.o :	../sound/%.c ../sound/%.h	        
		$(CC) -c $(CFLAGS) $(INCLUDES) $(DEFINES) $< -o $@
obj/%.o :	../sound/%.c      
		$(CC) -c $(CFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

obj/%.o :	../input_hw/%.c ../input_hw/%.h	        
		$(CC) -c $(CFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

obj/%.o :	../cart_hw/%.c ../cart_hw/%.h	        
		$(CC) -c $(CFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

obj/%.o :	../cart_hw/svp/%.c      
		$(CC) -c $(CFLAGS) $(INCLUDES) $(DEFINES) $< -o $@
obj/%.o :	../cart_hw/svp/%.c ../cart_hw/svp/%.h	        
		$(CC) -c $(CFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

obj/%.o :	../z80/%.c ../z80/%.h	        
		$(CC) -c $(CFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

obj/%.o :	../m68k/%.c ../m68k/%.h	        
		$(CC) -c $(CFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

obj/%.o :	./%.c ./%.h	        
		$(CC) -c $(CFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

obj/%.o :	../ntsc/%.c ../ntsc/%.h	        
		$(CC) -c $(CFLAGS) $(INCLUDES) $(DEFINES) $< -o $@
	        
obj/icon.o :	
		windres icon.rc $@

pack	:
		strip $(NAME)
		upx -9 $(NAME)	        

clean:
	rm -f $(OBJECTS) $(NAME)
