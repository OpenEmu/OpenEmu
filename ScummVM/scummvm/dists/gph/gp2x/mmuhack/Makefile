TARGET  = mmuhack
INCLUDE = -I/opt/gcc-3.4.4-glibc-2.3.6/arm-open2x-linux/sys-include
CFLAGS  = -O2 -DMODULE -D__KERNEL__ ${INCLUDE}
CC	= arm-open2x-linux-gcc

all: ${TARGET}.o

${TARGET}.o: ${TARGET}.c

clean:
	rm -rf ${TARGET}.o
