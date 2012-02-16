 ----------------------------------------------------------------------------
 tgemu                                 NEC PC-Engine / TurboGrafx-16 emulator
 ----------------------------------------------------------------------------

 Version 0.1
 by Charles MacDonald

 Table of contents
 ~~~~~~~~~~~~~~~~~
 1. What's new
 2. Controls
 3. Using the command line
 4. Source code
 5. What's missing
 6. Assistance needed
 7. Acknowledgements
 8. Contact
 9. Legal

 What's new
 ~~~~~~~~~~
 - Initial release

 Controls
 ~~~~~~~~
 Arrow keys     -   1P directional pad
 A, S, D, F     -   1P Button II, Button I, RUN, SELECT
 F1-F4          -   Set frameskip level (1-4)
 F9             -   Toggle FPS meter.
 F10            -   Toggle VSync polling.
 F11            -   Toggle speed throttling.
 F12            -   Make a PCX snapshot of the display.

 Using the command line
 ~~~~~~~~~~~~~~~~~~~~~~
 Run tgemu like so:

 pce file.[pce|zip] [-options]

 ZIP file loading will always load the first file in the archive.

 Portions of a file name that use spaces or special characters like single
 quotes need to be encased in double quotes. For instance:

 c:\pce roms\"Yo' Bro (U).pce"

 If you need to type more than 128 characters on the command line, you
 can either put options in a text file and add '@file.txt' to the
 command line, or put the options in a file called 'pce.cfg'. In either
 case, you must put one option on each line.

 Here is a list of valid options:

 -vdriver <s>        	 Select video driver (auto)
 -res <w> <h>        	 Specify display resolution (320x240)
 -depth <n>          	 Specify display depth (8)
 -auto <w> <h>           Enable automatic display switching (400x300)
 -blur <on|off>      	 Enable blur effect (16-bit color only)
 -scanlines <on|off> 	 Enable scanlines effect
 -scale <on|off>     	 Scale display to width of screen
 -vsync <on|off>     	 Enable vsync polling
 -throttle <on|off>  	 Enable speed throttling
 -fps <on|off>       	 Show FPS meter
 -skip <n>           	 Specify frame skip level (1=no frames skipped)
 -sound <on|off>     	 Enable sound output
 -sndcard <n>        	 Select sound card
 -sndrate <n>        	 Specify sound sample rate (8000-44100)
 -swap <on|off>      	 Swap left and right channels
 -wave <file.wav>    	 Log sound output to 'file.wav'
 -joy <s>            	 Select joystick driver (auto)
 -split <yes|no>     	 Split ROM image at 2 megabit boundary
 -flip <yes|no>      	 Bit-flip image
 -usa <yes|no>       	 Enable TurboGrafx-16 system detection

 tgemu will generate a file called 'pce.brm' for games that try to
 access the backup RAM to store game data. Currently only one save
 file is available for all games to use.

 Source code
 ~~~~~~~~~~~
 Like SMS Plus, I'm releasing the source code in hopes that tgemu will
 be ported to other platforms, and that people can learn a thing or two
 from it. (but not to be used almost verbatim in another program - that's
 what stopped the c2emu project)

 If you want to port tgemu to another platform, please download the
 source code available at my website, and read the instructions regarding
 how it may be used and to what extent.

 And most of all, please try your best to contact me so we can discuss
 the port. I like to know who's porting what, so there are no conflicts
 of interest.

 What's missing
 ~~~~~~~~~~~~~~
 - Sound stops in some games.
 - The VDC and CPU emulation may need improvement
 - Display rendering could be faster
 - Sprite overflow and collision interrupt emulation
 - Sprite to sprite and sprite to background priority
 - Sound emulation needs a lot of work
 - Multitap, mouse, 3 and 6 button gamepad emulation
 - Games with extra hardware like Tennokoe Bank, SF2:CE'
 - CD-ROM support

 Assistance needed
 ~~~~~~~~~~~~~~~~~
 I'm interested in obtaining a PC-Engine backup unit like the Magic
 Griffon (or similiar) and a copy of the Develo Book.

 Has anyone made homebrewn development hardware using EPROMs or an
 EPROM emulator? I'd like to hear more about it and what's involved.

 Are there any HuCard games that use the 3-button pad and/or mouse
 exclusively?

 Acknowledgements
 ~~~~~~~~~~~~~~~~
 - Bryan McPhail for the H6280 CPU emulation.
 - Cafe Noir for the Develo Book information.
 - Carlos Hasan for SEAL.
 - Chris MacDonald for translating documents and support.
 - Contributors to the tghack-list.
 - Contributors to the TG-Internals website.
 - Cowering for GoodPCE and related utilities.
 - Dave for DGEN, which I based the 16-bit blurring code on.
 - David Michel for the Magic Kit and Magic Engine.
 - David Shadoff for TGSIM and his help with the MESS PCE driver.
 - Emanuel Schleussinger for the VDC documentation.
 - Gilles Vollant for the unzip code.
 - Jean-loup Gailly and Mark Adler for ZLIB.
 - MAME for the SEAL interface code.
 - Paul Clifford for the excellent PSG documentation.
 - Richard Bannister for the Macintosh port, code, and advice.
 - The authors of the PNG specification for 8-bit palette setup ideas.

 Contact
 ~~~~~~~

 Please take the following in mind before you e-mail me:

 - If you don't like this program, don't use it.
 - I will not send you game images.

 Charles MacDonald
 E-mail: cgfm2@hotmail.com
 WWW: http://cgfm2.emuviews.com

 Legal
 ~~~~~
 The source code is distributed under the terms of the GNU General Public
 License.

 The SEAL interface code is taken from the MAME project, and terms of it's
 use are covered under the MAME license. (http://www.mame.net)

 The blur code is based on the 'CTV' sources from Dave's DGen emulator.
 (http://www.dtmnt.com)

