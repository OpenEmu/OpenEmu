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
 *
 */

#ifndef TOLTECS_ANIMATION_H
#define TOLTECS_ANIMATION_H

#include "toltecs/toltecs.h"
#include "toltecs/resource.h"

namespace Toltecs {

class AnimationPlayer {
public:
	AnimationPlayer(ToltecsEngine *vm);
	~AnimationPlayer();

	void start(uint resIndex);
	void nextFrame();
	int16 getStatus();
	uint16 getFrameNumber() const { return _frameNumber; }

	void saveState(Common::WriteStream *out);
	void loadState(Common::ReadStream *in);

//protected:
public:
	ToltecsEngine *_vm;

	// 262144
	byte *_animBuffer;

	uint16 _resIndex;

	uint16 _width, _height;
	uint16 _frameNumber, _frameCount;
	uint32 _keepFrameCounter;

	uint32 _curFrameSize;
	uint32 _nextFrameSize, _nextFrameOffset;

	uint32 _firstNextFrameOffset, _firstCurFrameSize, _firstNextFrameSize;

	void unpackFrame();

};

} // End of namespace Toltecs

#endif /* TOLTECS_ANIMATION_H */
