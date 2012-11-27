/*------------------------------------------------------------------------------
	$Id$

	key input code -- provides slightly higher level input forming

	Copyright (C) 2005
			Christian Auby (DesktopMan)
			Dave Murphy (WinterMute)

	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any
	damages arising from the use of this software.

	Permission is granted to anyone to use this software for any
	purpose, including commercial applications, and to alter it and
	redistribute it freely, subject to the following restrictions:

	1.	The origin of this software must not be misrepresented; you
		must not claim that you wrote the original software. If you use
		this software in a product, an acknowledgment in the product
		documentation would be appreciated but is not required.
	2.	Altered source versions must be plainly marked as such, and
		must not be misrepresented as being the original software.
	3.	This notice may not be removed or altered from any source
		distribution.

	$Log: keys.c,v $
	Revision 1.13  2006/01/12 09:10:47  wntrmute
	Added key repeat as suggested by pepsiman

	Revision 1.12  2005/11/27 12:30:25  wntrmute
	reverted to correct hardware REGisters

	Revision 1.11  2005/11/27 07:48:45  joatski
	Renamed REG_KEYINPUT and REG_KEYCNT back to KEYS and KEYS_CR, as the alternatives are defined in registers_alt.h.
	Changed function returns to uint32

	Revision 1.10  2005/11/03 23:38:49  wntrmute
	don't use enum for key function returns

	Revision 1.9  2005/10/13 16:30:11  dovoto
	Changed KEYPAD_BITS to a typedef enum, this resolved some issues with multiple redefinition of KEYPAD_BITS (although this error did not allways occur).

	Revision 1.8  2005/10/03 21:21:59  wntrmute
	use enum types

	Revision 1.7  2005/09/07 18:06:27  wntrmute
	use new register names

	Revision 1.6  2005/08/23 17:06:10  wntrmute
	converted all endings to unix

	Revision 1.5  2005/08/03 18:07:55  wntrmute
	don't use nds.h

	Revision 1.4  2005/07/25 02:31:07  wntrmute
	made local variables static
	added proper header to keys.h

	Revision 1.3  2005/07/25 02:19:01  desktopman
	Added support for KEY_LID in keys.c.
	Moved KEYS_CUR from header to source file.
	Changed from the custom abs() to stdlib.h's abs().

	Revision 1.2  2005/07/14 08:00:57  wntrmute
	resynchronise with ndslib


------------------------------------------------------------------------------*/

#include <stdlib.h>

#include <NDS/scummvm_ipc.h>
#include <nds/system.h>
#include <nds/arm9/input.h>


#define KEYS_CUR (( ((~REG_KEYINPUT)&0x3ff) | (((~IPC->buttons)&3)<<10) | (((~IPC->buttons)<<6) & (KEY_TOUCH|KEY_LID) ))^KEY_LID)

namespace DS {

static uint16 keys = 0;
static uint16 keysold = 0;
static uint16 keysrepeat = 0;

static u8 delay = 60, repeat = 30, count = 60;

static uint16 oldx = 0;
static uint16 oldy = 0;

void scanKeys(void) {
	keysold = keys;
	keys = KEYS_CUR;

	oldx = IPC->touchXpx;
	oldy = IPC->touchYpx;
    if (delay != 0) {
        if (keys != keysold) {
            count = delay;
            keysrepeat = keysDown();
        }
        count--;
        if (count == 0) {
            count = repeat;
            keysrepeat = keys;
        }
    }
}

uint32 keysHeld(void) {
	return keys;
}

uint32 keysDown(void) {
	return (keys ^ keysold) & keys;
}

uint32 keysDownRepeat(void) {
	uint32 tmp = keysrepeat;
    keysrepeat = 0;
    return tmp;
}

void keysSetRepeat( u8 setDelay, u8 setRepeat ) {
    delay = setDelay;
    repeat = setRepeat;
    count = delay;
    keysrepeat = 0;
}

uint32 keysUp(void) {
	return (keys ^ keysold) & (~keys);
}


}	// End of namespace DS
