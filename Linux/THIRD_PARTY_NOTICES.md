# Source and license notices

This fork retains the upstream source code and notices. It is a community
Linux port and is not endorsed by the original OpenEmu or emulator developers.

| Component | Source / license in this checkout |
| --- | --- |
| OpenEmu platform code and system definitions | `../OpenEmu/`; BSD notices in source headers; [upstream](https://github.com/OpenEmu/OpenEmu) |
| Gambatte core and resampler | `../Gambatte/COPYING`; GPL version 2; [OpenEmu core](https://github.com/OpenEmu/Gambatte-Core) |
| mGBA core | `../mGBA/LICENSE`; MPL 2.0 and per-file third-party notices; [OpenEmu core](https://github.com/OpenEmu/mGBA-Core) |
| Snes9x core | `../SNES9x/src/LICENSE`; Snes9x License and the component notices there; [OpenEmu core](https://github.com/OpenEmu/SNES9x-Core) |
| Qt for Python / PySide6 | Runtime dependency, separately installed; [licensing](https://doc.qt.io/qtforpython-6/licenses.html) |
| SDL2 | System runtime dependency, separately installed; [license](https://github.com/libsdl-org/SDL/blob/SDL2/LICENSE.txt) |
| System icons | Original OpenEmu artwork, read from `../OpenEmu/SystemPlugins/*/Images.xcassets`; original checkout and notices retained |
| OpenVGDB catalog and cover URLs | Downloaded from [OpenVGDB releases](https://github.com/OpenVGDB/OpenVGDB/releases), following OpenEmu's `OpenVGDB.swift` and `GameInfoHelper.swift` data model |
| Backup box art | Downloaded from [Libretro thumbnails](https://github.com/libretro-thumbnails/libretro-thumbnails); see that project's image credits |
| Libretro API header | `native/vendor/libretro.h`; the upstream MIT notice is retained; [canonical header](https://github.com/libretro/libretro-common/blob/master/include/libretro.h) |
| Optional Libretro cores | Downloaded on request from [Libretro's build server](https://buildbot.libretro.com/); each core retains its own license and copyright |

The Libretro host in `native/libretro.cpp` is new MIT-licensed frontend code.
Downloading a core does not change its license: see the relevant upstream
projects for [Gambatte](https://github.com/libretro/gambatte-libretro),
[SameBoy](https://github.com/libretro/SameBoy),
[mGBA](https://github.com/libretro/mgba),
[Snes9x](https://github.com/libretro/snes9x),
[Snes9x 2010](https://github.com/libretro/snes9x2010), and
[bsnes](https://github.com/libretro/bsnes).
Downloaded core binaries are stored in the user's library rather than bundled
with this source fork. The frontend does not bundle RetroArch or third-party
BIOS files.

Downloaded game box art belongs to its respective copyright holders. It is
cached in the user's library and is not covered by the MIT license on new Linux
code. The catalog and thumbnail indexes are fetched on demand; this fork does
not bundle the downloaded databases or a collection of covers. The library
screenshot demonstrates a downloaded cover.

The new Linux C++ SNES adapter retains the full OpenEmu BSD notice in
`native/snes9x.cpp`. `native/gambatte.cpp` derives from
`../Gambatte/GBGameCore.mm` (Copyright 2015 OpenEmu Team) and is marked GPL-2.0-only
for its integration with Gambatte. `native/mgba.c` derives from
`../mGBA/src/platform/openemu/mGBAGameCore.m` (Copyright 2016 Jeffrey Pfau) and is
marked MPL-2.0. Their original BSD notices follow.

## OpenEmu Gambatte adapter notice

Copyright (c) 2015, OpenEmu Team

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
3. Neither the name of the OpenEmu Team nor the names of its contributors may
   be used to endorse or promote products derived from this software without
   specific prior written permission.

THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
EVENT SHALL OpenEmu Team BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

## OpenEmu mGBA adapter notice

Copyright (c) 2016, Jeffrey Pfau

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ''AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

## Diagnostic cartridges

The GB/GBC, GBA and SNES test ROMs in `demos/` are original diagnostic programs
generated for this port from the corresponding sources in `tests/`. Their
source and generated bytes are CC0-1.0. The SNES hardware behavior was checked
against the SNESdev [SPC700 boot protocol](https://snes.nesdev.org/wiki/Booting_the_SPC700)
and [controller-reading reference](https://snes.nesdev.org/wiki/Controller_reading).
No commercial ROM images or third-party BIOS files are included in `demos/`.
