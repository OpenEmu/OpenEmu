#www.freedo.org
#The first and only working 3DO multiplayer emulator.
#
#The FreeDO licensed under modified GNU LGPL, with following notes:
#
#*   The owners and original authors of the FreeDO have full right to develop closed source derivative work.
#*   Any non-commercial uses of the FreeDO sources or any knowledge obtained by studying or reverse engineering
#    of the sources, or any other material published by FreeDO have to be accompanied with full credits.
#*   Any commercial uses of FreeDO sources or any knowledge obtained by studying or reverse engineering of the sources,
#    or any other material published by FreeDO is strictly forbidden without owners approval.
#
#The above notes are taking precedence over GNU LGPL in conflicting situations.
#
#Project authors:
#
#Alexander Troosh
#Maxim Grishin
#Allen Wright
#John Sammons
#Felix Lazarev
##########################################################################################################################

#source files

SRC = Clio.cpp Madam.cpp bitop.cpp arm.cpp DSP.cpp Iso.cpp quarz.cpp SPORT.cpp vdlp.cpp XBUS.cpp DiagPort.cpp _3do_sys.cpp

OBJ = $(SRC:.cpp=.o)

OUT = libfreedo.a

#include dirs
INCLUDES = -I. -I/usr/local/include

#C++ flags
CCFLAGS = -g

#compiler
CCC = g++

#libraries
LIBS = -L/usr/local/lib

#compile flags
LDFLAGS = -g

.SUFFIXES: .cpp

default:	dep $(OUT)

.cpp.o:
	$(CCC) $(INCLUDES) $(CCFLAGS) -c $< -o $@

$(OUT): $(OBJ)
	ar rcs $(OUT) $(OBJ)

depend:	dep

dep:
	makedepend -- $(CFLAGS) -- $(INCLUDES) $(SRC)

clean:
	rm -f $(OBJ) $(OUT) Makefile.bak Makefile~



