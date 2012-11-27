/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Data structures used by the fader and flasher processes
 */

#ifndef TINSEL_FADERS_H		// prevent multiple includes
#define TINSEL_FADERS_H

#include "tinsel/dw.h"	// for SCNHANDLE
#include "tinsel/tinsel.h"

namespace Tinsel {

/**
 * Number of iterations in a fade out.
 */
// FIXME: There seems to be some confusion in Tinsel 2 whether this should be 9 or 6
#define COUNTOUT_COUNT 6

/*----------------------------------------------------------------------*\
|*                      Fader Function Prototypes                       *|
\*----------------------------------------------------------------------*/

// usefull palette faders - they all need a list of palettes that
//				should not be faded. This parameter can be
//				NULL - fade all palettes.

void FadeOutMedium(SCNHANDLE noFadeTable[]);
void FadeOutFast(SCNHANDLE noFadeTable[]);
void FadeInMedium(SCNHANDLE noFadeTable[]);
void FadeInFast(SCNHANDLE noFadeTable[]);
void PokeInTagColor();

} // End of namespace Tinsel

#endif		// TINSEL_FADERS_H
