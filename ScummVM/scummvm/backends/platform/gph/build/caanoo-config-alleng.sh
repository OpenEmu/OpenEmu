#!/bin/sh

echo Quick script to make running configure all the time less painful
echo and let all the build work be done from the backend/build folder.

# Assume Caanoo toolchain/build env and source it.
. /opt/arm-caanoo/environment-setup

# Edit the configure line to suit.
cd ../../../..
./configure --backend=caanoo --disable-mt32emu --host=caanoo \
  --disable-alsa --disable-flac \
  --disable-nasm --disable-vorbis --disable-hq-scalers \
  --with-sdl-prefix=/opt/arm-caanoo/arm-none-linux-gnueabi/usr/bin \
  --enable-tremor --with-tremor-prefix=/opt/arm-caanoo/arm-none-linux-gnueabi/usr \
  --enable-zlib --with-zlib-prefix=/opt/arm-caanoo/arm-none-linux-gnueabi/usr \
  --enable-mad --with-mad-prefix=/opt/arm-caanoo/arm-none-linux-gnueabi/usr \
  --enable-png --with-png-prefix=/opt/arm-caanoo/arm-none-linux-gnueabi/usr \
  --enable-all-engines --enable-vkeybd --enable-plugins --default-dynamic

echo Generating config for Caanoo complete. Check for errors.
