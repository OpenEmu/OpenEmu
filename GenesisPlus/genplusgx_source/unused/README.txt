 ----------------------------------------------------------------------------
 Genesis Plus  
 ----------------------------------------------------------------------------

 based on the original version 1.2a
 by Charles Mac Donald
 WWW: http://cgfm2.emuviews.com

 version 1.4
 by Eke-Eke
 WWW: http://code.google.com/p/genplus-gx


 What's New
 ----------

 see CHANGELOG


 Features 
 ---------

    * NTSC (60Hz) & PAL (50Hz) hardware emulation
    * highly accurate 68000 & Z80 CPU emulation
    * highly accurate VDP emulation (HBLANK, DMA, FIFO, HV interrupts, undocumented registers, display mid-line changes…)
    * cycle-accurate YM2612 emulation (FM synthesis is done at the original frequency, using FIR resampling)
    * cycle-accurate chip synchronization (68000/Z80/YM2612/SN76489)
    * basic hardware latency emulation (VDP/68k, Z80/68k)
    * full overscan area (horizontal & vertical colored borders) emulation (optional)
    * accurate TMSS model emulation incl. internal BIOS support (optional)
    * Blargg's software NTSC filters support (optional)
    * PICO emulation (partial) 
    * 3-buttons & 6-buttons controllers support
    * Sega Team Player & EA 4-Way Play multitap adapters support
    * Sega Menacer & Konami Justifier lightguns support
    * Sega Mouse support
    * J-Cart adapter support(Micro Machines & Pete Sampras series, Super Skidmarks)
    * SVP DSP emulation (Virtua Racing)
    * SRAM support (up to 64kB)
    * serial EEPROM emulation (all known chips)
    * ROM bankswitch hardware support (Super Street Fighter 2)
    * ROM/SRAM bankswitch hardware support (Phantasy Star 4, Legend of Thor, Sonic the Hedgehog 3)
    * emulates most banking & copy protection devices used in unlicensed/pirate cartridges
    * native Game Genie & Action Replay hardware emulation
    * Lock-On hardware emulation
    * support for ROM image up to 10MB (Ultimate MK3 hack) 


 Usage
 -----

 The Windows version runs windowed in a 16-bit desktop with 48Hz sound using SDL but
 without joystick support.

 The DOS version has most of the functionality from SMS Plus, such
 as audio, joysticks, etc.

 Windows/DOS controls:

 Arrow Keys -   Directional pad
 A,S,D,F    -   1P gamepad, buttons A, B, C, StartTab        -   Hard Reset 
 Esc        -   Exit program

 F7	    -	Load Savestate (game.gpz)
 F8	    -	Save Savestate (game.gpz)
 F9	    -	Toggle CPU clock: PAL(50hz)/NTSC(60hz)
 F10	    -   Soft Reset
 F11	    -   Toggle Border emulation (vertical only)
 F12        -   Toggle Player # (keyboard/mouse)

 DOS only:

 F1-F4      -   Set frameskip level (F1 = no skip ... F4 = skip 3 frames)

 Windows only:

 F4         -   Toggle Audio (ONLY when Turbo mode is OFF)
 F6	    -   Toggle Turbo mode (ONLY when Audio is OFF)


 The mouse can be used for lightgun games, PICO games and when the Sega Mouse is activated.

 A SRAM file (game.srm) is automatically saved on exit and loaded on startup.

 DOS details:

 You can only support a second player if you are using a joystick driver
 that supports more than one joystick. (e.g. Sidewinder, dual pads, etc.)

 Type 'gen -help' on the command line for a list of useful options.

    -res <x> <y>    set the display resolution.
    -vdriver <n>    specify video driver.
    -depth <n>      specify color depth. (8, 16)
    -scanlines      use scanlines effect.
    -scale          scale display to full resolution. (slow)
    -vsync          wait for vertical sync before blitting.
    -sound          enable sound. (force speed throttling)
    -sndrate <n>    specify sound rate. (8000, 11025, 22050, 44100)
    -sndcard <n>    specify sound card. (0-7)
    -swap           swap left and right stereo output.
    -joy <s>        specify joystick type.

 Here is a list of all the video drivers you can pass as a parameter
 to the '-vdriver' option:

    auto, safe, vga, modex, vesa2l, vesa3, vbeaf

 Here is a list of all the joystick drivers you can pass as a parameter
 to the '-joy' option:

    auto, none, standard, 2pads, 4button, 6button, 8button, fspro, wingex,
    sidewinder, gamepadpro, grip, grip4, sneslpt1, sneslpt2, sneslpt3,
    psxlpt1, psxlpt2, psxlpt3, n64lpt1, n64lpt2, n64lpt3, db9lpt1, db9lpt2,
    db9lpt3, tglpt1, tglpt2, tglpt3, wingwar, segaisa, segapci, segapci2

 You can put any commandline option into a plain text file which should
 be called "gen.cfg". Put one option per line, please. Command line options
 will override anything in the configuration file.

 Currently the zip loading code can manage a zipfile where the game
 image is the first thing in it. If you try to open a huge archive of
 games, only the first will be played.

 Credits and Acknowlegements
 ---------------------------

 I would like to thank Omar Cornut, Christian Schiller, and Chris MacDonald
 for their invaluable help and support with this project.

 Richard Bannister for the Macintosh port and all of the code fixes and
 suggestions that have made Genesis Plus a better program.
 (http://www.bannister.org)

 Extra features, emulation accuracy & game compatibility improvments  by Eke-Eke
 (from the Gamecube/Wii port: http://code.google.com/p/genplus-gx )

 The Genesis emulator authors: Bart Trzynadlowski, Quintesson, Steve Snake,
 James Ponder, Stef, Gerrie, Sardu, AamirM

 The regular people and many lurkers at segadev and spritesmind.net.

 The MAME team for the CPU and sound chip emulators.

 Maxim for his SN76489 emulator.

 Nemesis for his researches about the YM2612 and VDP.
 
 Notaz for his SVP emulator.

 Tasco Deluxe for his documentation of Realtec mapper.

 Haze for his reverse-engineering of most unlicensed games protection.

 Shay Green (Blargg) for his FIR Resampler & NTSC Video Filter.

 Neil C for his 3-Band EQ implementation.

 Jean-Loup Gailly and Mark Adler for their useful zip library.


 Contact
 -------

 Charles MacDonald
 E-mail: cgfm2@hotmail.com
 WWW: http://cgfm2.emuviews.com

 Eke-Eke
 E-mail: eke_eke31@yahoo.fr
 WWW: http://code.google.com/p/genplus-gx


 Legal
 -----

 Copyright (C) 1999, 2000, 2001, 2002, 2003  Charles MacDonald
 Copyright (C) 2006-2010  Eke-Eke

 The source code is distributed under the terms of the GNU General Public
 License.

 The Z80 CPU emulator, 68K CPU emulator, and the YM2612 emulation code
 are taken from the MAME project, and terms of their use are covered under
 the MAME license. (http://www.mame.net)

