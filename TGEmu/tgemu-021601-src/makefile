# tgemu - NEC PC-Engine emulator
# (c) 2000, 2001  Charles MacDonald

# -DLSB_FIRST - Leave undefined for big-endian processors.
# -DFAST_MEM  - Use fast, but not technically correct, memory access.
# -DDOS       - Enable DOS/x86 specific code.
# -DDEBUG     - Enable error message logging.

CC	=	gcc
AS	=	nasm -f coff
LDFLAGS	=
FLAGS	=	-I. -Icpu -Idos -Isound \
		-Werror -Wall \
		-mpentium -O6 -fomit-frame-pointer \
		-DLSB_FIRST -DDOS -DFAST_MEM
LIBS	=	-lalleg -laudio -lz
OBJ	=       obj/h6280.oa	\
		obj/pce.o	\
		obj/render.o	\
		obj/vdc.o	\
		obj/vce.o	\
		obj/psg.o	\
		obj/system.o    \
		obj/unzip.o     \
		obj/fileio.o
OBJ	+=	obj/dos.o	\
		obj/blur.o	\
		obj/sealintf.o	\
		obj/config.o	\
		obj/wave.o	\
		obj/error.o

EXE	=	pce.exe

all	:	$(EXE)

$(EXE)	:	$(OBJ)
		$(CC) -o $(EXE) $(OBJ) $(LIBS) $(LDFLAGS)
	        
obj/%.oa :	cpu/%.c cpu/%.h
		$(CC) -c $< -o $@ $(FLAGS)

obj/%.o : 	%.c %.h
		$(CC) -c $< -o $@ $(FLAGS)
	        
obj/%.o :	dos/%.s
		$(AS) $< -o $@
	        
obj/%.o :	sound/%.c sound/%.h	        
		$(CC) -c $< -o $@ $(FLAGS)
	        
obj/%.o :	cpu/%.c cpu/%.h	        
		$(CC) -c $< -o $@ $(FLAGS)

obj/%.o :	dos/%.c dos/%.h	        
		$(CC) -c $< -o $@ $(FLAGS)
	        
pack	:
		strip $(EXE)
		upx -1 $(EXE)	        

clean	:	        
		rm -f obj/*.o
		rm -f *.bak
		rm -f *.exe
		rm -f *.log
		rm -f *.wav
		rm -f *.zip
cleancpu :		
		rm -f obj/*.oa

makedir :
		mkdir obj
	        
archive:	        
		pk -dir -add -max \
		-excl=rom -excl=obj -excl=src -excl=out -excl=zip \
		-excl=doc -excl=snap -excl=new -excl=bak \
		pcesrc.zip *.*
	        
#
# end of makefile
#
