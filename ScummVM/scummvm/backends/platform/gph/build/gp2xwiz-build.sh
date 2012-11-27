#!/bin/bash

echo Quick script to make building all the time less painful.

# Set the paths up here to support the build.

export PATH=/opt/open2x/gcc-4.1.1-glibc-2.3.6/arm-open2x-linux/bin:$PATH
export PATH=/opt/open2x/gcc-4.1.1-glibc-2.3.6/bin:$PATH
export CXX=arm-open2x-linux-g++
export CC=arm-open2x-linux-gcc
export CXXFLAGS="-mcpu=arm926ej-s -mtune=arm926ej-s"
export ASFLAGS=-mfloat-abi=soft

cd ../../../..

echo Building ScummVM for GP2X Wiz.
make

echo Build for GP2X Wiz - complete - Please check build logs.
