/*
 * GlideHQ (Texture enhancer library for Glide64)
 * Version:  1.5
 *
 * Copyright (C) 2007  Hiroshi Morii aka KoolSmoky   All Rights Reserved.
 * Email koolsmoky(at)users.sourceforge.net
 * Web   http://www.3dfxzone.it/koolsmoky
 *
 * this is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * this is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Make; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */

About:
This is a realtime texture enhancer library with hi-resolution texture
pack support for Glide64 (http://glide64.emuxhaven.net). Traditional and
non-traditional techniques have been used to achieve speed and high image
quality even on a 9 year old 3Dfx Voodoo2.

Although the 3Dfx Glide3x texture format naming conventions are used, the
library can be expanded for generic use.

Supported:
OS: 32bit Linux and MS Windows
Enhancers: Hq4x, Hq2x, Hq2xS, Lq2x, Lq2xS, Super2xSai, x2
Filters: Smooth (1,2,3,4), Sharp (1,2)
Compressors: FXT1, S3TC
Input formats:  GR_TEXFMT_ALPHA_8,
                GR_TEXFMT_RGB_565,
                GR_TEXFMT_ARGB_1555,
                GR_TEXFMT_ARGB_4444,
                GR_TEXFMT_ARGB_8888,
                GR_TEXFMT_ALPHA_INTENSITY_44,
                GR_TEXFMT_ALPHA_INTENSITY_88
Output formats: Same as input unless compression or hires packs are used.
Hires texture packs: Rice format (Jabo and GlideHQ format coming later)

Acknowledgments:
I hope you enjoy GlideHQ (texture enhancer library for Glide64). Greatest
thanks to Gonetz for making this happen in his busy time. We've rushed
everything to share the eye-candy with all of you N64 emulation fans. I
would also like to thank a great friend of mine, Daniel Borca for providing
the texture compression code, Maxim Stepin (hq2x 4x), and Derek Liauw Kie Fa
(2xSaI) for the filtering engines, Rice for his N64 graphics plugin source
code, and Mudlord for the hq2xS lq2xS code. GlideHQ also uses the boost C++
libraries, zlib general purpose compression library, and the Portable Network
Graphics library. Thanks to all the developers for making them available. And
special thanks to the Glide64 beta testing crew. Without their feedbacks
this library would not have seen daylight. Thank you all.

The source code for GlideHQ is released in hopes that it will be improved.
I know the coding is not on par after so much late night caffeine boosts.
If you have suggestions or modifications, please feel free to post them on
the Glide64 forum at emuxhaven.

Porting the library to other platforms should not be so hard. The coding is
done with cross platform compatibility in mind and will build with GCC and
GNU make. Currently supported are 32bit Linux and MS Windows.

If you are looking for driver updates for your 3Dfx Interactive Inc. gfx 
card, grab them from the forums at http://www.3dfxzone.it/enboard/
Unbelievable as it seems, drivers are still being updated after 6 years
from 3Dfx's demise.

I know N64 rules, anyone up for PSX? :))

-KoolSmoky

References:
[1] R.W. Floyd & L. Steinberg, An adaptive algorithm for spatial grey scale,
    Proceedings of the Society of Information Display 17, pp75-77, 1976
[2] Ken Turkowski, Filters for Common Resampling Tasks, Apple Computer 1990
    http://www.worldserver.com/turk/computergraphics/ResamplingFilters.pdf
[3] Don P. Mitchell and Arun N. Netravali, Reconstruction Filters in Computer
    Graphics, SIGGRAPH '88, Proceedings of the 15th annual conference on
    Computer graphics and interactive techniques, pp221-228, 1988
[4] J. F. Kaiser and W. A. Reed, Data smoothing using low-pass digital
    filters, Rev. Sci. instrum. 48 (11), pp1447-1457, 1977
[5] Maxim Stepin, hq4x Magnification Filter, http://www.hiend3d.com/hq4x.html
[6] Derek Liauw Kie Fa, 2xSaI, http://elektron.its.tudelft.nl/~dalikifa
[7] Dirk Stevens, Eagle engine http://www.retrofx.com/rfxtech.html
[8] 3DFX_texture_compression_FXT1 and EXT_texture_compression_s3tc extension
    specs from the OpenGL Extension Registry. http://oss.sgi.com/projects/
    ogl-sample/registry/
