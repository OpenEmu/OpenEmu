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

#ifndef TEENAGENT_ANIMATION_H
#define TEENAGENT_ANIMATION_H

#include "common/stream.h"
#include "teenagent/surface.h"

namespace TeenAgent {

class Animation  {
public:
	uint16 id, x, y;
	bool loop, paused, ignore;

	enum Type {kTypeLan, kTypeVaria, kTypeInventory};

	Animation();
	~Animation();

	void load(Common::SeekableReadStream &, Type type = kTypeLan);
	void free();

	Surface *firstFrame();
	Surface *currentFrame(int dt);
	uint16 currentIndex() const { return index; }
	void resetIndex() { index = 0; }

	bool empty() const { return frames == NULL; }
	void restart();

	//uint16 width() const { return frames? frames[0].w: 0; }
	//uint16 height() const { return frames? frames[0].h: 0; }

protected:
	byte *data;
	uint16 dataSize;

	uint16 framesCount;
	Surface *frames;
	uint16 index;
};

} // End of namespace TeenAgent

#endif
