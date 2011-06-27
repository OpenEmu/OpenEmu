 ----------------------------------------------------------------------------
 SMS Plus
 ----------------------------------------------------------------------------

 A free, open-source Sega Master System and Game Gear emulator.

 Version 1.2
 by Charles MacDonald

 What's New
 ----------

 [Version 1.2]

 - Fixed smptab allocation error.
 - Various source code cleanups.
 - Minor tweaks to sync with Mac source changes.
 - Added more support for PAL hardware.
 - Added partial support for Codemasters games.
 - Fixed some aspects of ROM loading.
 - Fixed SRAM restore bug in save states.
 - Fixed viewport size check when going from extended to normal mode, which
   fixes Fantastic Dizzy opening screen and V counter test program.
 - Split up VDP emulation into SMS/GG/MD specific parts.
 - Added support for different port I/O port memory maps.
 - Fixed mapper restore bug in save states.
 - Fixed UI key management in dos/main.c
 - Improved HV counter emulation.
 - Added dump VRAM keyboard shortcut. (Alt+V)
 - Split up DOS code into different modules.
 - Fixed EXT connector and related I/O port 2 behavior.
 - Added GG I/O register emulation.
 - Changed SRAM management.
 - Modified use of reset/poweron/poweroff functions.
 - Added configuration via CRC for some Codemasters games.
 - Fixed implementation of GG input port.
 - Added cycle counter feature to Z80 emulator.
 - Added shutdown functions to other modules.
 - Mapped 2nd player inputs to keyboard numeric pad in dos/main.c
 - Fixed reset button handling in dos/main.c
 - Moved Z80 port handlers to memz80.c
 - Fixed console type assign in loadrom.c (removed TYPE_* defines)
 - Added support for I/O chip disable control.
 - Rewrote I/O chip emulation.
 - Fixed pixel LUT to handle sprite collision behind high priority BG tiles.
 - Added emulation of sprite overflow status flag.
 - Added 'granularity' member to bitmap struct and fixed DOS blur code.
 - Fixed FM sound restore on state load / FM emulator change.
 - Corrected screen blanking width.
 - Removed sprite limit disable feature.
 - Added support for extended display modes.
 - Added partial support for PAL display timing.
 - Removed BMP_* macros, replaced with bitmap.viewport.* variables.
 - Removed Y's (J) rendering hack that was problematic in other games.
 - Added error logging routines.
 - Removed Game Gear specific rendering speedups.
 - Replaced cart.type with sms.console, replaced access with IS_GG macro.
 - Renamed INPUT_SOFT_RESET to INPUT_RESET. Use for SMS games only.
 - Removed INPUT_HARD_RESET, use system_reset() instead.
 - Modified vdp_ctrl_w() to update address register LSB during leading write.
 - Changed path length in wram/state/snap routines to PATH_MAX in DOS code.
 - Added define to specify message length in DOS code.
 - Added r/w handlers for FM detection latch access, renamed fm* -> fmunit*
 - Added territory/console members to struct sms (removed sms.country).
   Removed TYPE_* and replaced with TERRITORY_* enums.
 - Fixed FM register restore during state load when sound is disabled.
 - Updated memory system to support 1K pages.
 - Updated zlib to 1.2.1 and unzip.c to 1.0
 - Moved sound management out of system.c into sound/sound.c,fmintf.c
 - Moved state management out of system.c into state.c
 - Rearranged header file include order in system.h
 - Added support for MAME YM2413 emulator
 - Abstracted FM sound chip interface to support both YM2413 emulators
 - Updated timeline in dos/main.c and system.c
 - Removed SSL logging
 - Fixed path length in loadrom.c to MAX_PATH from limits.h
 - Added library version display option to dos/main.c
 - Moved file I/O code to fileio.c
 - Fixed loadrom.c to support 16K ROM images
 - Updated documentation and porting instructions
 - Modified EMU2413 to have update function for 2-channel output
 - Modified dos\config.c to ensure parameters are left during option parsing
 - Modified YM2413 emulator to check for NULL pointer during shutdown
 - Cleaned up variable names in snd struct
 - Added default mixer callback
 - Made sound_shutdown() free memory
 - Modified sound_init() to allow re-initialization
 - Cleaned up system.h, sms.h, vdp.h
 - Optimized color expansion for paletteized 8-bit format in render.c
 - Added Maxim's SN76489 emulator, removed the old one
 - Added YM2413 context management
 - Fixed PSG/FM state save/load behavior

 * Save state format has changed in this release, and will probably change
   in future updates. Old states from version 0.9.x are not interchangable
   with the new version 1.x states.

 [Version 0.9.4b]

 - Made a few minor changes to the source code.
 - Added keyboard shortcut to dump work RAM.
 - Fixed FPS state display.
 - Fixed GG palette handling.
 - Improved tile caching.
 - Made SMS/GG palette brighter.
 - Updated Z80 CPU emulator.

 [Version 0.9.4a]

 - A few cleanups so SMS Plus compiles with no errors or warnings.
 - Use of MMX code is now automatic but can still be disabled manually.
 - Replaced FM sound emulation with Mitsutaka Okazaki's EMU2413 library.
 - Fixed parts of the VDP emulation.
 - Removed support for PSX port.

 [Version 0.9.3]

 - Stereo Game Gear sound.
 - Save states, 10 per game.
 - Sound logging, output is .GYM compatible.
 - Optimized memory access for improved speed.
 - Screen snapshots in PCX format.
 - Stereo swap option for left and right speakers.
 - Added FPS meter.
 - Added some option toggle keys for in-game configuration.
 - Definable video driver.
 - Tweaked display modes for full-screen gameplay.
 - More of the usual internal changes and bug fixes.

 [Version 0.9.2]

 - Digital YM2413 FM sound emulation. Drums sound real good, too.
 - System territory switch, default is overseas (Europe / USA)
 - Made some internal changes and clean-ups to the source code.
 - Revised document on porting to be actually useful.
 - Adjusted version number to stay in sync with the Macintosh port.
 - Support for ZIPped game images and games with 512-byte headers.
 - Screen expansion now works for 16-bit displays.
 - Fixed some centering problems with screen expansion on GG games.
 - Sound output using SEAL should be a bit clearer.
 - Configuration file support.

 [Version 0.9]

 Everything has been rewritten from scratch. The emulation is much more
 accurate, as a result compatability has improved greatly.

 The DOS port has a lot of nice features added, including 16-bit color,
 display blurring, digital sound emulation, and scanlines, which were the
 most requested items.

 Also new is a Sony Playstation port, though it's highly experimental
 and has some serious performance issues.

 SMS Plus is now free software; the source code is available for anyone
 to use as they see fit, and is distributed under the terms of the GNU
 General Public License.

 Usage
 -----

 (DOS)

 You'll need at least a Pentium 133, a VGA compatible display card,
 and optionally a joystick and sound card.

 Controls are as follows:

 Arrow Keys -   Directional pad
 a          -   Button II
 s          -   Button I
 Enter      -   Start (GG) / PAUSE (SMS)
 Tab        -   Soft reset (SMS) / Hard reset (GG)
 Delete     -   Hard reset
 Esc/End    -   Exit program
 0-9        -   Select save state slot
 Alt+W      -   Dump work RAM
 ~          -   Switch between EMU2413 and YM2413 FM sound chip emulators.

 F1-F4      -   Set frameskip level (F1 = no skip ... F4 = skip 3 frames)
 F5         -   Save state file from current slot
 F6         -   Cycle through state slots (0-9)
 F7         -   Save state file to current slot
 F8         -   Make PCX screen snapshot
 F9         -   Toggle VSync
 F10        -   Toggle speed throttling
 F11        -   Toggle FPS meter

 You can only support a second player if you are using a joystick driver
 that supports more than one joystick. (e.g. Sidewinder, dual pads, etc.)

 Type 'sp -help' on the command line for a list of useful options.

    -res <x> <y>    set the display resolution.
    -vdriver <n>    specify video driver.
    -depth <n>      specify color depth. (8, 16)
    -blur           blur display. (16-bit color only)
    -scanlines      use scanlines effect.
    -tweak          force tweaked 256x192 or 160x144 8-bit display.
    -scale          scale display to full resolution. (slow)
    -expand         force 512x384 or 400x300 zoomed display.
    -nommx          disable use of MMX instructions.
    -novga          disable use of VGA vertical scaling with '-expand'.
    -vsync          wait for vertical sync before blitting.
    -throttle       limit updates to 60 frames per second.
    -fps            show FPS meter.
    -sound          enable sound. (force speed throttling)
    -sndrate <n>    specify sound rate. (8000, 11025, 22050, 44100)
    -sndcard <n>    specify sound card. (0-7)
    -swap           swap left and right stereo output.
    -joy <s>        specify joystick type.
    -jp             use Japanese console type.
    -fm             required to enable YM2413 sound.
    -info           show library versions.
    -codies         force Codemasters mapper

 Here is a list of all the video drivers you can pass as a parameter
 to the '-vdriver' option:

    auto, safe, vga, modex, vesa2l, vesa3, vbeaf

 Here is a list of all the joystick drivers you can pass as a parameter
 to the '-joy' option:

    auto, none, standard, 2pads, 4button, 6button, 8button, fspro, wingex,
    sidewinder, gamepadpro, grip, grip4, sneslpt1, sneslpt2, sneslpt3,
    psxlpt1, psxlpt2, psxlpt3, n64lpt1, n64lpt2, n64lpt3, db9lpt1, db9lpt2,
    db9lpt3, tglpt1, tglpt2, tglpt3, wingwar, segaisa, segapci, segapci2

 If you use the expand option without scanlines, and the display looks
 squashed vertically, then also use the '-novga' switch.

 You can disable MMX use with the '-nommx' switch.

 You can put any commandline option into a plain text file which should
 be called "sp.cfg". Put one option per line, please. Command line options
 will override anything in the configuration file.

 Battery back-up RAM and save states are saved to the same directory as the
 game you loaded. This means running games off write-only media will not
 work. These files are named after the name of the image itself, not the
 name of the zipfile archive in the case of zip loading.

 Currently the zip loading code can manage a zipfile where the game
 image is the first thing in it. If you try to open a huge archive of
 games, only the first will be played.

 Credits and Acknowledgments
 ---------------------------

 Dedicated to Chris MacDonald.

 Thanks to:

 All Allegro contributors, Bero, Carlos Hasan, Dave, Eric Quinn,
 Frank Hughes, Flavio Morsoletto, Gilles Volant, Hiromitsu Shioya, Jon,
 Jean-loup Gailly, James McKay, Jarek Burczynski, Kreed, Mark Adler,
 Micheal Cunanan, Marcel de Kogel, Marat Fayzullin, Mitsutaka Okazaki,
 Maxim, Nyef, Nick Jacobson, Omar Cornut, Paul Leaman, Ricardo Bittencourt,
 Richard Mitton, Richard Talbot-Watkins, Sean Young, Tatsuyuki Satoh,
 the MAME team, and the S8-DEV forum members.

 Richard Bannister for the Macintosh port. (www.bannister.org)
 Richard Teather for the Win32 port. (smsplus.vintagegaming.com)
 Caz Jones for the BeOS port. (http://www.infernal.currantbun.com)
 Cyx for the SDL port. (http://membres.lycos.fr/cyxdown/smssdl/)
 Ulrich Hecht for the Linux port. (http://www.emulinks.de/emus/)
 ss_teven for the Sega Dreamcast port. (no homepage URL)
 Dagolar for the Sega Saturn port. (http://phemusat.tripod.com/)
 Aj0 for the GP32 port. (http://ajo.thinknerd.com/gp32/sms32/files/)
 ? for the X-Box port. (http://xport.xb-power.com/smsplus.html)
 ? for the Sony Playstation 2 port. (http://nik.napalm-x.com/)

 All of those responsible for many console, mobile phone, and PDA ports
 of SMS Plus.

 The artists who worked on Red Zone and Sub Terrania for the cool font
 used in previous DOS ports.

 I'd also like to thank everybody who contributed information, bug reports,
 and gave their comments and ideas.

 Contact
 -------

 Charles MacDonald
 E-mail: cgfm2 at hotmail dot com
 WWW: http://cgfm2.emuviews.com

 Legal
 -----

 SMS Plus is Copyright (C) 1998-2004  Charles MacDonald

 The source code is distributed under the terms of the GNU General Public
 License.

 The SN76489 emulator is written by Maxim.
 (http://mwos.cjb.net)

 The EMU2413 emulator is written by Mitsutaka Okazaki.
 (http://www.angel.ne.jp/~okazaki/ym2413/)

 The Z80 CPU and YM2413 emulator, and SEAL interface code are taken from the
 MAME project, and terms of their use are covered under the MAME license.
 (http://www.mame.net)

 The blur code is based on the 'CTV' sources from Dave's DGen emulator.
 (http://www.dtmnt.com)

