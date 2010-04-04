--------------------------------------------------------------
glitchNES 0.1 by NO CARRIER
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

asm6.exe      - ASM6 assembler by loopy - http://home.comcast.net/~olimar/NES/
compile.bat   - Batch file to compile glitchNES
geo.chr       - CHR file by Alex Mauer  - http://www.headlessbarbie.com
glitchnes.asm - Source code for glitchNES
gpl.txt       - GNU General Public License
readme.txt    - You're reading it

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

glitchNES is an NES ROM image. It will work in Nestopia (see above)
and other NES emulators. It has also been tested on NTSC NES hardware
and a Subor famiclone using both EEPROM development carts and the
RetroZone PowerPak.

Each button on controller one does something. The directional pad
controls scrolling while the select, start, B, and A buttons control
writes to the screen. All of this can be changed, of course, by
editing the source code. I recommend Context with the 6502 highlighter
for easy source code manipulation. Even easier than editing the source
code is changing the tileset.

You can use YY-CHR or a tile editor of your choice to edit the NES
ROM image once compiled. Replacing the tiles in the ROM will completely
alter the graphics, but the program will remain the same.

--------------------------------------------------------------
STILL TO COME
--------------------------------------------------------------

Future versions of this software will include controller two usage to
automate and loop effects, PowerPad support for dancing chiptune
musicians, sprites, glitchy sounds driven by graphics on the screen,
and much more.

--------------------------------------------------------------
VERSION HISTORY
--------------------------------------------------------------

0.1 - 03.20.2009 - Initial release

