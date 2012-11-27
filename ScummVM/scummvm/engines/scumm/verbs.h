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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef SCUMM_VERBS_H
#define SCUMM_VERBS_H

#include "common/scummsys.h"
#include "common/rect.h"

namespace Scumm {

/**
 * The area in which some click (or key press) occurred and which is passed
 * to the input script.
 */
enum ClickArea {
	kVerbClickArea = 1,
	kSceneClickArea = 2,
	kInventoryClickArea = 3,
	kKeyClickArea = 4,
	kSentenceClickArea = 5
};

enum {
	kTextVerbType = 0,
	kImageVerbType = 1
};

struct VerbSlot {
	Common::Rect curRect;
	Common::Rect oldRect;
	uint16 verbid;
	uint8 color, hicolor, dimcolor, bkcolor, type;
	uint8 charset_nr, curmode;
	uint16 saveid;
	uint8 key;
	bool center;
	uint8 prep;
	uint16 imgindex;
};

enum VerbsV0 {
	kVerbNone    = 0,
	kVerbOpen    = 1,
	kVerbClose   = 2,
	kVerbGive    = 3,
	kVerbTurnOn  = 4,
	kVerbTurnOff = 5,
	kVerbFix     = 6,
	kVerbNewKid  = 7,
	kVerbUnlock  = 8,
	kVerbPush    = 9,
	kVerbPull    = 10,
	kVerbUse     = 11,
	kVerbRead    = 12,
	kVerbWalkTo  = 13,
	kVerbPickUp  = 14,
	kVerbWhatIs  = 15
};

enum VerbPrepsV0 {
	kVerbPrepNone   = 0,
	kVerbPrepIn     = 1,
	kVerbPrepWith   = 2,
	kVerbPrepOn     = 3,
	kVerbPrepTo     = 4,
	kVerbPrepObject = 0xFF // prep depends on object (USE)
};

} // End of namespace Scumm

#endif
