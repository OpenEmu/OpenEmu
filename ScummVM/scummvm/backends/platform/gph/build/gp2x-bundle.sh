#!/bin/sh

echo Quick script to make building a distribution of the GP2X port more consistent.

# Set the paths up here to support the build.

export PATH=/opt/open2x/gcc-4.1.1-glibc-2.3.6/arm-open2x-linux/bin:$PATH
export PATH=/opt/open2x/gcc-4.1.1-glibc-2.3.6/bin:$PATH
export CXX=arm-open2x-linux-g++
export CC=arm-open2x-linux-gcc
export CXXFLAGS=-march=armv4t
export LDFLAGS=-static
export ASFLAGS=-mfloat-abi=soft

cd ../../../..

echo Building ScummVM for GP2X Wiz.

make gp2x-bundle
