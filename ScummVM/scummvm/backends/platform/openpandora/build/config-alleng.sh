#!/bin/sh

echo Quick script to make running configure all the time less painful
echo and let all the build work be done from the backend/build folder.

. /usr/local/angstrom/arm/environment-setup

CROSS_COMPILE=arm-angstrom-linux-gnueabi-
export CROSS_COMPILE

# Export the tool names for cross-compiling
export CXX=arm-angstrom-linux-gnueabi-g++
export CPPFLAGS=-I/usr/local/angstrom/arm/arm-angstrom-linux-gnueabi/usr/include
export LDFLAGS=-L/usr/local/angstrom/arm/arm-angstrom-linux-gnueabi/usr/lib

export DEFINES=-DNDEBUG

# Edit the configure line to suit.
cd ../../../..
./configure --backend=openpandora --host=openpandora --disable-nasm \
  --with-sdl-prefix=/usr/local/angstrom/arm/arm-angstrom-linux-gnueabi/usr/bin \
  --disable-vorbis --enable-tremor --with-tremor-prefix=/usr/local/angstrom/arm/arm-angstrom-linux-gnueabi/usr \
  --enable-zlib --with-zlib-prefix=/usr/local/angstrom/arm/arm-angstrom-linux-gnueabi/usr \
  --enable-mad --with-mad-prefix=/usr/local/angstrom/arm/arm-angstrom-linux-gnueabi/usr \
  --enable-png --with-png-prefix=/usr/local/angstrom/arm/arm-angstrom-linux-gnueabi/usr \
  --enable-all-engines --enable-plugins --default-dynamic

echo Generating config for OpenPandora complete. Check for errors.
