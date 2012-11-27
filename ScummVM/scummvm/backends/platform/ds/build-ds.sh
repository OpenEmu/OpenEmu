#!/bin/sh

echo "FIXME: feature disabled in configure"
exit 1

#
# build-ds.sh -- script for building a ds build with every usable dynamic engine plugin

make clean

#Set up a static build with only engines usable by DS enabled
./configure --host=ds --disable-debug --disable-all-engines --enable-scumm --enable-sky --enable-queen --enable-agos --enable-gob --enable-cine --enable-agi --enable-kyra --enable-lure --enable-parallaction --enable-made --enable-cruise

make clean

make

#Dump all symbols used in this garbage-collected static build into a file
rm -f ds.syms
arm-eabi-objdump -t scummvm.elf > ds.syms

make clean

#Set up a dynamic build with only engines usable by the DS enabled
./configure --host=ds --enable-plugins --default-dynamic --disable-debug --disable-all-engines --enable-scumm --enable-sky --enable-queen --enable-gob --enable-cine --enable-agos --enable-agi --enable-kyra --enable-lure --enable-parallaction --enable-made --enable-cruise

make clean

#Make this final build, which is selectively stripped with the assistance of the ds.syms file that was generated earlier
make
