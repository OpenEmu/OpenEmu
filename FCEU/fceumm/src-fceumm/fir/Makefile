CC	= gcc
%.h:	%.coef
	cat $< | ./toh > $@ || true

all:            floogie c44100ntsc.h c48000ntsc.h c96000ntsc.h c44100pal.h c48000pal.h c96000pal.h

#c44100ntsc.h:    c44100ntsc.coef
#c48000ntsc.h:    c48000ntsc.coef
#c96000ntsc.h:    c96000ntsc.coef
#c44100pal.h:	c44100pal.coef
#c48000pal.h:	c48000pal.coef
#c96000pal.h:	c96000pal.coef

floogie:	toh.o
		gcc -o toh toh.o
