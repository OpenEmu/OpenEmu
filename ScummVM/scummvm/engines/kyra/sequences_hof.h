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

#ifndef KYRA_SEQUENCES_HOF_H
#define KYRA_SEQUENCES_HOF_H

#include "kyra/kyra_v2.h"

namespace Kyra {

struct HoFSequence {
	const char *wsaFile;
	const char *cpsFile;
	uint16 flags;
	uint8 fadeInTransitionType;
	uint8 fadeOutTransitionType;
	int16 stringIndex1;
	int16 stringIndex2;
	uint16 startFrame;
	uint16 numFrames;
	uint16 duration;
	uint16 xPos;
	uint16 yPos;
	uint16 timeout;
};

struct HoFNestedSequence {
	const char *wsaFile;
	const FrameControl *wsaControl;
	uint16 flags;
	uint16 startframe;
	uint16 endFrame;
	uint16 frameDelay;
	uint16 x;
	uint16 y;
	uint16 fadeInTransitionType;
	uint16 fadeOutTransitionType;
};

struct HoFSeqData {
	const HoFSequence *seq;
	int numSeq;
	const HoFNestedSequence *nestedSeq;
	int numNestedSeq;
};

struct HoFSeqItemAnimData {
	int16 itemIndex;
	uint16 y;
	const uint16 *frames;
};

} // End of namespace Kyra

#endif
