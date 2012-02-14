/*
WiiHandy : Port of the Handy Emulator for the Wii

Copyright (C) 2011
raz0red (www.twitchasylum.com)

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
*/

#ifndef WII_HANDY_MAIN_H
#define WII_HANDY_MAIN_H

#include "system.h"
#include "errorhandler.h"

/* Handy declarations */
extern Uint32 *mpLynxBuffer;
extern CSystem *mpLynx;

/* Wii declarations */
extern int emulationRunning;

/*
 * Initializes WiiHandy
 */
extern void wii_handy_init();

/*
 * Loads the specified ROM
 *
 * gamefile The ROM to load
 */
extern void wii_handy_load_rom( char* gamefile );

/*
 * The main WiiHandy emulator loop
 */
extern void wii_handy_emu_loop();

#endif
