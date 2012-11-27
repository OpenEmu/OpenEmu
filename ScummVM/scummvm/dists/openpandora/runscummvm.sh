#!/bin/sh

# Make sure any extra libs not in the firmware are pulled in.
LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:../lib
export LD_LIBRARY_PATH

# Ensure we have a folder to store save games on the SD card.
mkdir saves

# make a runtime dir, just incase it creates anything in CWD
mkdir runtime
cd runtime

../bin/scummvm --fullscreen  --gfx-mode=2x --config=../scummvm.config  --themepath=../data
