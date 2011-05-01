This README covers the Ogg Vorbis 'Tremor' integer playback codec
source as of date 2002 09 02, version 1.0.0.

                            ******

The C source in this package will build on any ANSI C compiler and
function completely and properly on any platform.  The included build
system assumes GNU build system and make tools (m4, automake,
autoconf, libtool and gmake).  GCC is not required, although GCC is
the most tested compiler.  To build using GNU tools, type in the
source directory:

./autogen.sh
make

Currently, the source implements playback in pure C on all platforms
except ARM, where a [currently] small amount of assembly (see
asm_arm.h) is used to implement 64 bit math operations and fast LSP
computation.  If building on ARM without the benefit of GNU build
system tools, be sure that '_ARM_ASSEM_' is #defined by the build
system if this assembly is desired, else the resulting library will
use whatever 64 bit math builtins the compiler implements.

No math library is required by this source.  No floating point
operations are used at any point in either setup or decode.  This
decoder library will properly decode any past, current or future
Vorbis I file or stream.

                           ********

The build system produces a static and [when supported by the OS]
dynamic library named 'libvorbisidec'.  This library exposes an API
nearly identical to the BSD reference library's 'libvorbisfile',
including all the features familiar to users of vorbisfile.  This API
is similar enough that the proper header file to include is named
'ivorbisfile.h' [included in the source build directory].  Lower level
libvorbis-style headers and structures are in 'ivorbiscodec.h'
[included in the source build directory]. A simple example program,
ivorbisfile_example.c, can be built with 'make example'.

                           ********

Detailed Tremor API Documentation begins at doc/index.html

Monty
xiph.org
