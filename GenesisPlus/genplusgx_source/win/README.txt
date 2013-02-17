 ----------------------------------------------------------------------------
 Genesis Plus  
 ----------------------------------------------------------------------------

 based on the original version 1.3
 by Charles Mac Donald
 WWW: http://cgfm2.emuviews.com

 version 1.6
 backported from Genesis Plus GX
 by Eke-Eke
 WWW: http://code.google.com/p/genplus-gx


 What's New
 ----------

 see CHANGELOG.txt


 Features 
 ---------

    * accurate SG-1000, Mark-III, Master System (I & II), Game Gear, Genesis & Mega Drive emulation (incl. backwards compatibility modes)
    * NTSC (60Hz) & PAL (50Hz) hardware emulation
    * highly accurate 68000 & Z80 CPU emulation
    * highly accurate VDP emulation (all rendering modes, HBLANK, DMA, FIFO, HV interrupts, undocumented registers, display mid-line changes…)
    * cycle-accurate YM2612 & YM2413 emulation (FM synthesis is done at the original frequency, using FIR resampling)
    * cycle-accurate chip synchronization (68000/Z80/YM2612/SN76489)
    * basic hardware latency emulation (VDP/68k, Z80/68k)
    * full overscan area (horizontal & vertical colored borders) emulation (optional)
    * accurate TMSS model emulation incl. internal BIOS support (optional)
    * Blargg's software NTSC filters support (optional)
    * PICO emulation (partial)
    * 2-buttons, 3-buttons & 6-buttons controllers emulation
    * Sega Team Player & EA 4-Way Play multitap adapters emulation
    * Sega Light Phaser, Menacer & Konami Justifier lightguns emulation
    * Sega Mouse emulation
    * Sega Activator emulation
    * XE-1AP emulation
    * Sega Paddle & Sports Pad emulation
    * Terebe Oekaki emulation   
    * J-Cart adapter support(Micro Machines & Pete Sampras series, Super Skidmarks)
    * SVP DSP emulation (Virtua Racing)
    * SRAM support (up to 64kB)
    * I2C (24Cxx) and MicroWire (93C46) EEPROM emulation (all known chips)
    * ROM bankswitch hardware emulation (Super Street Fighter 2)
    * SRAM bankswitch hardware emulation (Phantasy Star 4, Legend of Thor, Sonic the Hedgehog 3)
    * emulation of banking & copy protection devices used in all known unlicensed/pirate cartridges
    * emulation of all known Master System & Game Gear cartridge mappers
    * native Game Genie & Action Replay hardware emulation
    * Lock-On hardware emulation
    * support for ROM image up to 10MB (Ultimate MK3 hack) 


 Usage
 -----

 The Windows version runs windowed in a 16-bit desktop with 48Hz sound using SDL but
 without joystick support.

 
 Controls
 -----

 Arrow Keys -   Directional pad
 A/Q,S,D,F  -   buttons A, B(1), C(2), START
 W,X,C,V    -   buttons X, Y, Z, MODE if 6-buttons controller is enabled
 Tab        -   Hard Reset 
 Esc        -   Exit program

 F2         -   Toggle Fullscreen/Windowed mode
 F4         -   Toggle Audio (Turbo mode must be disabled first)
 F6	        -   Toggle Turbo mode (Audio must be disabled first)
 F7	        -	Load Savestate (game.gpz)
 F8	        -	Save Savestate (game.gpz)
 F9	        -	Toggle VDP mode: PAL(50hz)/NTSC(60hz)
 F10	    -   Soft Reset
 F11	    -   Toggle Border emulation
 F12        -   Toggle Player # (test only)

 
 The mouse is used for lightguns, Sega Mouse, PICO & Terebi Oekaki tablet (automatically detected when loading supported game).

 A SRAM file (game.srm) is automatically saved on exit and loaded on startup.


 Credits and Acknowlegements
 ---------------------------

 Original code by Charles MacDonald

 Modified Genesis Plus GX code by Eke-Eke (http://code.google.com/p/genplus-gx )

 The Genesis emulator authors: Bart Trzynadlowski, Steve Snake, Stef, Notaz, AamirM

 The regular people at spritesmind.net and smspower.org.

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

 Eke-Eke
 E-mail: ekeeke31@gmail.com
 WWW: http://code.google.com/p/genplus-gx


 Legal
 -----

 See license.txt for licensing terms.

