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

#ifndef LURE_ANIMSEQ_H
#define LURE_ANIMSEQ_H

#include "lure/screen.h"

namespace Lure {

enum AnimAbortType {ABORT_NONE, ABORT_END_INTRO, ABORT_NEXT_SCENE};

struct AnimSoundSequence {
	uint16 numFrames;
	uint8 adlibSoundId;
	uint8 rolandSoundId;
	uint8 channelNum;
};

class AnimationSequence {
private:
	bool _isEGA;
	uint16 _screenId;
	Palette &_palette;
	MemoryBlock *_decodedData;
	MemoryBlock *_lineRefs;
	byte *_pPixels, *_pLines;
	byte *_pPixelsEnd, *_pLinesEnd;
	const AnimSoundSequence *_soundList;
	int _frameDelay;

	AnimAbortType delay(uint32 milliseconds);
	void egaDecodeFrame(byte *&pPixels);
	void vgaDecodeFrame(byte *&pPixels, byte *&pLines);
public:
	AnimationSequence(uint16 screenId, Palette &palette,  bool fadeIn, int frameDelay = 7,
		const AnimSoundSequence *soundList = NULL);
	~AnimationSequence();

	AnimAbortType show();
	bool step();
};

} // End of namespace Lure

#endif
