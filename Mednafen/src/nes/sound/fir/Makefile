CC	= gcc
%.h:	%.coef
	cat $< | ./toh > $@ || true

all:            floogie ntsc.h pal.h

floogie:	toh.o
		gcc -o toh toh.o
