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
 */

#ifndef SCUMM_BOXES_H
#define SCUMM_BOXES_H

#include "common/rect.h"

namespace Scumm {

#define SIZEOF_BOX_V0 5
#define SIZEOF_BOX_V2 8
#define SIZEOF_BOX_V3 18
#define SIZEOF_BOX 20
#define SIZEOF_BOX_V8 52

typedef enum {
	kBoxXFlip		= 0x08,
	kBoxYFlip		= 0x10,
	kBoxIgnoreScale	= 0x20,
	kBoxPlayerOnly	= 0x20,
	kBoxLocked		= 0x40,
	kBoxInvisible	= 0x80
} BoxFlags;

struct BoxCoords {			/* Box coordinates */
	Common::Point ul;
	Common::Point ur;
	Common::Point ll;
	Common::Point lr;
};

int getClosestPtOnBox(const BoxCoords &box, int x, int y, int16& outX, int16& outY);

} // End of namespace Scumm

#endif
