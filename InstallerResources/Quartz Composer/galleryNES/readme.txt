--------------------------------------------------------------
galleryNES 0.1 by NO CARRIER
8bitpeoples Research & Development - http://www.8bitpeoples.com
--------------------------------------------------------------

Copyright 2009 Don Miller
For more information, visit: http://www.no-carrier.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

--------------------------------------------------------------
INCLUDED FILES
--------------------------------------------------------------

asm6.exe       - ASM6 assembler by loopy - http://home.comcast.net/~olimar/NES/
compile.bat    - Batch file to compile galleryNES
edit_pics.bat  - Batch file to edit individual screens
gallerynes.asm - Source code for galleryNES
geo.chr        - CHR file by Alex Mauer  - http://www.headlessbarbie.com
geo.pal        - Default palette file
gpl.txt        - GNU General Public License
NESpalette.png - NES palette found on deviantART, by Erik Red
*.nam files    - Default NES screen / nametable files
readme.txt     - You're reading it

--------------------------------------------------------------
FILES YOU'LL NEED IN THE SAME FOLDER
--------------------------------------------------------------

name.exe       - From Pin Eight Software's NES Tools, used to edit screens
               - http://www.pineight.com/pc/p8nes.zip
alleg40.dll    - Required for running Allegro applications (like name.exe)
               - http://www.pineight.com/pc/alleg40.zip

--------------------------------------------------------------
RECOMMENDED SOFTWARE
--------------------------------------------------------------

YY-CHR   - Tile editor  - http://www.briansemu.com/yymarioed/
Context  - Text editor  - http://www.contexteditor.org/
Nestopia - NES emulator - http://nestopia.sourceforge.net/

--------------------------------------------------------------
RECOMMENDED HARDWARE
--------------------------------------------------------------

PowerPak - NES flash cart         - http://www.retrousb.com
ReproPak - NES reproduction board - http://www.retrousb.com

--------------------------------------------------------------
USAGE
--------------------------------------------------------------

galleryNES is an NES ROM image. It will work in Nestopia (see above)
and other NES emulators. It has also been tested on NTSC NES hardware
and a Subor famiclone using both EEPROM development carts and the
RetroZone PowerPak.

Controlling the program is simple. Left and right cycle through the
pictures. This can be changed, of course, by editing the source code.
I recommend Context with the 6502 highlighter for easy source code
manipulation. Even easier than editing the source code is changing
the tileset.

You can use YY-CHR or a tile editor of your choice to edit the geo.chr
file. You do not have to use the included tiles, but they may be a good
start if you don't feel like drawing your own. Replacing the tiles in
the ROM will completely alter the graphics, but the program will remain
the same. You need to use the same tileset across all of the screens /
nametables.

Each of the 10 included screens / nametables are .nam files can be edited
by running edit_pics.bat. However, make sure you have name.exe and
alleg40.dll in the same folder. After running the batch file, just choose
a screen number to edit. See the name.exe documentation that comes with the
NES Tools package for more info on using the nametable / screen editor.
Don't worry about the palettes you see in the program, as they are controlled
by the gallerynes.asm file.

If you open up gallerynes.asm in a text editor and scroll to the bottom,
you will see a large block of numbers under the label "palette". Take a
look at this example line:

$0F,$00,$10,$30,$0F,$05,$26,$30,$0F,$13,$23,$33,$0F,$1C,$2B,$39

Break that string of digits up into four groups of four to better visualize
the four NES palettes:

---palette #1---  ---palette #2---  ---palette #3---  ---palette #4---

$0F,$00,$10,$30   $0F,$05,$26,$30   $0F,$13,$23,$33   $0F,$1C,$2B,$39

Now, match up those first four with the included NESpalette.png file:

$0F = black (the recommended black by the NESDEV community, long story..)
$00 = dark grey
$10 = grey
$30 = white

Not too bad, right? So, edit the pictures with the batch file, and edit the
palette in the gallerynes.asm. One catch: there has to be a background color
that is constant across all 4 palettes - you can see above that is $0F, or
black. That means you get a total of 13 colors per screen. Wait, one more
catch: you can only one palette per each 16x16 pixel area. You'll see that
when you start working with the nametable / screen editor.

Finally, make sure you understand that palette #0 is linked to pic0.nam,
palette #1 is linked to pic1.nam, etc. You can have a different palette for
each screen / nametable, BUT all screens use the same tileset: geo.chr by
default.

I guess that is about it. Enjoy!

--------------------------------------------------------------
VERSION HISTORY
--------------------------------------------------------------

0.1 - 05.29.2009 - Initial release