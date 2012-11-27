Build instructions:

1. Install BADA SDK (requires free registration):

 http://developer.bada.com/apis/index.do

2. Install Cygwin:

 http://www.cygwin.com/

 Add the following to your cygwin .bash_profile:

 alias mmake=/cygdrive/c/MinGW/bin/mingw32-make.exe
 export BADA_SDK=/cygdrive/c/bada/1.2.1
 export ARM_BIN=c:/bada/1.2.1/Tools/Toolchains/ARM/bin
 export CPPFLAGS="-fpic -fshort-wchar -mcpu=cortex-a8 -mfpu=vfpv3 \
                  -mfloat-abi=hard -mlittle-endian -mthumb-interwork -Wno-psabi \
                  -fno-strict-aliasing -fno-short-enums"
 export LDFLAGS="-nostdlib -lc-newlib -lm-newlib -LC:/bada/1.2.1/Model/Wave_LP1/Target"
 #export PATH=${BADA_SDK}/Tools/Toolchains/Win32/bin:${PATH}
 export PATH=${BADA_SDK}/Tools/Toolchains/ARM/bin:~/utils:${PATH}
 alias gcc=${ARM_BIN}/arm-samsung-nucleuseabi-gcc.exe
 alias ar=${ARM_BIN}/arm-samsung-nucleuseabi-ar.exe

 The following were added to ~/utils for zlib:

 ar:
 #!/bin/sh
 ${ARM_BIN}/arm-samsung-nucleuseabi-ar.exe $*

 gcc:
 #!/bin/sh
 ${ARM_BIN}/arm-samsung-nucleuseabi-gcc.exe $*

3. Build dependencies

 zlib, libogg, libvorbis, libmad, FLAC

 see: "Building the libraries" under:
 http://wiki.scummvm.org/index.php/Compiling_ScummVM/MinGW#Building_the_libraries
 for instructions on how to obtain these modules

 3.1 For Target-Release configure ogg and mad with:

     ./configure --host=arm-samsung-nucleuseabi --disable-shared

     when building vorbis and flac:

     ./configure --host=arm-samsung-nucleuseabi --disable-shared --with-ogg=c:/cygwin/usr/local

 3.2 for each module, after a successful configure, add the following
     to the generated config.h (gzguts.h for zlib)

     #undef __MINGW32__
     #undef _WIN32
     #include "c:/src/scummvm/backends/platform/bada/portdefs.h"

 3.3 Additional post configure edits:

   - removed -fforce-mem from the libMAD Makefile
   - in libvorbis/lib/Makefile comment lines with vorbis_selftests
   - edit libFLAC/Makefile ... CFLAGS = $(OGG_CFLAGS)

 Note: you can ignore the ranlib errors when doing make install.

4. Build the ScummVM base library:

  ./configure --host=bada --enable-release

  To target the Win32 simulator:

 ./configure --host=bada --enable-debug

5. Build the front end application using BADA-Ide:

 Copy the scummvm/dists/bada folder into a clean directory
 outside of the scummvm package. Start the BADA IDE then
 choose this folder as the eclipse workspace. Click
 Project / Build.

Links:

A short turorial on implementing OpenGL ES 1.1 in BADA:
  http://forums.badadev.com/viewtopic.php?f=7&t=208

HelvB14 font files:
 http://www.cl.cam.ac.uk/~mgk25/ucs-fonts.html
 http://www.cl.cam.ac.uk/~mgk25/download/ucs-fonts-75dpi100dpi.tar.gz

 Then run the following command:
 $ ./ucs2any.pl 100dpi/helvB14.bdf MAPPINGS/8859-1.TXT iso8859-1 \
   MAPPINGS/8859-2.TXT iso8859-2 MAPPINGS/8859-3.TXT iso8859-3
