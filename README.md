OpenEmu Linux
=============

![alt text](http://openemu.org/img/intro-md.png "OpenEmu Screenshot")

OpenEmu Linux is an unofficial Linux port and derivative of the [OpenEmu](https://github.com/OpenEmu/OpenEmu) project.

The project began with the OpenEmu codebase as its foundation, with the goal of bringing OpenEmu's polished, library-focused game emulation experience to Linux. Since then, substantial portions of the original codebase have been modified, replaced, or rewritten to remove macOS-specific dependencies and support a Linux-focused architecture.

OpenEmu Linux is not an official release of OpenEmu and is not affiliated with or maintained by the upstream OpenEmu project.

Architecture
------------

The original OpenEmu project uses macOS technologies such as Cocoa, Metal, Core Animation, and a modular game-engine plugin architecture.

OpenEmu Linux keeps the same general goal of providing a unified game library and frontend while adapting the underlying architecture for Linux. The current Linux port uses:

* Python for the frontend and user interface
* C++ for the native integration and emulation layer
* SQLite for the game library and metadata database
* Standard libretro / RetroArch-compatible cores for emulation
* Linux-native platform integration in place of macOS-specific frameworks

Because the Linux port uses standard libretro cores rather than OpenEmu's original game-engine plugin system, emulator cores do not need to be specially modified for OpenEmu Linux.

Supported Systems and Emulator Projects
---------------------------------------

OpenEmu Linux is intended to support the same broad range of systems through compatible libretro cores. The emulator projects linked below are retained from the original OpenEmu README as references to the emulators and back ends associated with those systems.

* Atari 2600 ([Stella](https://github.com/stella-emu/stella))
* Atari 5200 ([Atari800](https://github.com/atari800/atari800))
* Atari 7800 ([ProSystem](https://gitlab.com/jgemu/prosystem))
* Atari Lynx ([Mednafen](https://mednafen.github.io))
* ColecoVision ([CrabEmu](https://sourceforge.net/projects/crabemu/))
* Famicom Disk System ([Nestopia](https://gitlab.com/jgemu/nestopia))
* Game Boy / Game Boy Color ([Gambatte](https://gitlab.com/jgemu/gambatte))
* Game Boy Advance ([mGBA](https://github.com/mgba-emu/mgba))
* GameCube ([Dolphin](https://github.com/dolphin-emu/dolphin))
* Game Gear ([Genesis Plus](https://github.com/ekeeke/Genesis-Plus-GX))
* Intellivision ([Bliss](https://github.com/jeremiah-sypult/BlissEmu))
* NeoGeo Pocket ([Mednafen](https://mednafen.github.io))
* Nintendo (NES) / Famicom ([FCEUX](https://github.com/TASEmulators/fceux), [Nestopia](https://gitlab.com/jgemu/nestopia))
* Nintendo 64 ([Mupen64Plus](https://github.com/mupen64plus))
* Nintendo DS ([DeSmuME](https://github.com/TASEmulators/desmume))
* Odyssey² / Videopac+ ([O2EM](https://sourceforge.net/projects/o2em/))
* PC-FX ([Mednafen](https://mednafen.github.io))
* SG-1000 ([Genesis Plus](https://github.com/ekeeke/Genesis-Plus-GX))
* Sega 32X ([picodrive](https://github.com/notaz/picodrive))
* Sega CD / Mega CD ([Genesis Plus](https://github.com/ekeeke/Genesis-Plus-GX))
* Sega Genesis / Mega Drive ([Genesis Plus](https://github.com/ekeeke/Genesis-Plus-GX))
* Sega Master System ([Genesis Plus](https://github.com/ekeeke/Genesis-Plus-GX))
* Sega Saturn ([Mednafen](https://mednafen.github.io))
* Sony PSP ([PPSSPP](https://github.com/hrydgard/ppsspp))
* Sony PlayStation ([Mednafen](https://mednafen.github.io))
* Super Nintendo (SNES) ([BSNES](https://github.com/bsnes-emu/bsnes), [Snes9x](https://github.com/snes9xgit/snes9x))
* TurboGrafx-16 / PC Engine ([Mednafen](https://mednafen.github.io))
* TurboGrafx-CD / PCE-CD ([Mednafen](https://mednafen.github.io))
* Vectrex ([VecXGL](https://github.com/james7780/VecXGL))
* Virtual Boy ([Mednafen](https://mednafen.github.io))
* WonderSwan ([Mednafen](https://mednafen.github.io))

Project Status
--------------

OpenEmu Linux is under active development. Because the project is replacing significant portions of OpenEmu's macOS-specific architecture, some features may be incomplete, experimental, or behave differently from upstream OpenEmu.

The goal is to preserve the parts of the OpenEmu experience that make it useful as a unified game library while using an architecture that is practical for Linux and standard libretro cores.

Current priorities include:

* A clean, library-focused game interface
* Game metadata and cover-art management
* Multiple emulated systems in a single application
* Standard libretro core support
* Controller support and configuration
* Minimal emulator-specific setup for normal use
* Linux-native behavior and integration

Relationship to OpenEmu
-----------------------

This project is derived from the original OpenEmu source code. It began as a Linux port of OpenEmu, but substantial portions of the application have been modified or rewritten as the project has moved away from macOS-specific frameworks and OpenEmu's original core-plugin architecture.

Upstream OpenEmu:

* [OpenEmu website](https://openemu.org)
* [OpenEmu GitHub repository](https://github.com/OpenEmu/OpenEmu)

This repository should not be considered an official Linux release of OpenEmu.

Requirements
------------

OpenEmu Linux currently targets Linux systems.

The project requires:

* Python
* A C++ development/runtime environment
* SQLite
* Compatible libretro cores
* Required Linux development libraries and runtime dependencies

Exact distribution-specific package requirements and minimum versions will be documented as the Linux build system stabilizes.

Building
--------

Building OpenEmu Linux requires the Python dependencies used by the frontend, a C++ toolchain for the native integration layer, SQLite development support, and the required Linux libraries for the current build.

The original OpenEmu macOS/Xcode build requirements do not apply to this Linux port.

Credits and Licensing
---------------------

OpenEmu Linux is based on the work of the OpenEmu project and its contributors.

Portions of this project derived from OpenEmu remain subject to the applicable OpenEmu copyright and license terms. Individual libretro cores, emulator projects, and third-party libraries may use their own licenses. Refer to the corresponding project and license files for details.
