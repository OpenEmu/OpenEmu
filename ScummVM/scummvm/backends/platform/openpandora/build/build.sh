#!/bin/sh

echo Quick script to make building all the time less painful.

. /usr/local/angstrom/arm/environment-setup

CROSS_COMPILE=arm-angstrom-linux-gnueabi-
export CROSS_COMPILE

# Export the tool names for cross-compiling
export CXX=arm-angstrom-linux-gnueabi-g++
export CC=arm-angstrom-linux-gnueabi-gcc
export DEFINES=-DNDEBUG

cd ../../../..

echo Building ScummVM/OpenPandora.
make

echo Build for OpenPandora complete - Please check build logs.
