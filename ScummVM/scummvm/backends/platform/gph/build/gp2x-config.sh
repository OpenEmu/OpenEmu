#!/bin/bash

echo Quick script to make running configure all the time less painful
echo and let all the build work be done from the backend/build folder.

# Set the paths up here to generate the config.

PATH=/opt/open2x/gcc-4.1.1-glibc-2.3.6/arm-open2x-linux/bin:$PATH
PATH=/opt/open2x/gcc-4.1.1-glibc-2.3.6/bin:$PATH

# Export the tool names for cross-compiling
export CXX=arm-open2x-linux-g++
export CXXFLAGS=-march=armv4t
export CPPFLAGS=-I/opt/open2x/gcc-4.1.1-glibc-2.3.6/include
export LDFLAGS=-L/opt/open2x/gcc-4.1.1-glibc-2.3.6/lib
export DEFINES=-DNDEBUG

# Edit the configure line to suit.
cd ../../../..
./configure --backend=gp2x --disable-mt32emu --host=gp2x \
  --disable-flac --disable-nasm --disable-hq-scalers \
  --with-sdl-prefix=/opt/open2x/gcc-4.1.1-glibc-2.3.6/bin \
  --enable-tremor --with-tremor-prefix=/opt/open2x/gcc-4.1.1-glibc-2.3.6 \
  --enable-zlib --with-zlib-prefix=/opt/open2x/gcc-4.1.1-glibc-2.3.6 \
  --enable-mad --with-mad-prefix=/opt/open2x/gcc-4.1.1-glibc-2.3.6 \
  --enable-vkeybd

# --enable-plugins --default-dynamic
# --disable-release --enable-debug  

echo Generating config for GP2X complete. Check for errors.
