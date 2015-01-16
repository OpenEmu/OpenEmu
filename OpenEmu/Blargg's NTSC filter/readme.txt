snes_ntsc 0.2.2: SNES NTSC Video Filter
---------------------------------------
This library filters a Super NES image to match what a TV would show,
allowing an authentic image in an emulator. It uses a highly optimized
algorithm to perform the same signal processing as an NTSC decoder in a
TV, giving very similar pixel artifacts and color bleeding. The usual
picture controls can be adjusted: hue, saturation, contrast, brightness,
and sharpness. Additionally, the amount of NTSC chroma and luma
artifacts can be reduced, allowing an image that corresponds to
composite video (artifacts), S-video (color bleeding only), RGB (clean
pixels), or anywhere inbetween.

The output is scaled to the proper horizontal width, leaving it up the
emulator to simply double the height. An optional even/odd field merging
feature is provided to reduce flicker when the host display's refresh
rate isn't 60 Hz. Specialized blitters can be easily written using a
special interface, allowing customization of input and output pixel
formats, optimization for the host platform, and efficient scanline
doubling.

Blitting a 256x240 source image to a 602x240 pixel 16-bit RGB memory
buffer at 60 frames per second uses 8% CPU on a 2.0 GHz Athlon 3500+ and
40% CPU on a 10-year-old 400 MHz G3 PowerMac. The library requires 4MB
of memory.

Author  : Shay Green <gblargg@gmail.com>
Website : http://www.slack.net/~ant/
Forum   : http://groups.google.com/group/blargg-sound-libs
License : GNU Lesser General Public License (LGPL)
Language: C or C++


Getting Started
---------------
Build a program from demo.c, snes_ntsc.c, and the SDL multimedia library
(see http://libsdl.org/). Run it with "test.bmp" in the same directory
and it should show the filtered image. See demo.c for more.

See snes_ntsc.txt for documentation and snes_ntsc.h for reference. Post to
the discussion forum for assistance.


Files
-----
readme.txt          Essential information
snes_ntsc.txt        Library documentation
changes.txt         Changes made since previous releases
license.txt         GNU Lesser General Public License

benchmark.c         Measures frame rate and processor usage of library
demo.c              Displays and saves NTSC filtered image
demo_impl.h         Internal routines used by demo
test.bmp            Test image for demo

snes_ntsc_config.h   Library configuration (modify as needed)
snes_ntsc.h          Library header and source
snes_ntsc.c
snes_ntsc_impl.h

-- 
Shay Green <gblargg@gmail.com>
