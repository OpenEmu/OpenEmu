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

#ifndef COMPOSER_GRAPHICS_H
#define COMPOSER_GRAPHICS_H

#include "common/rect.h"
#include "graphics/surface.h"

namespace Composer {

class ComposerEngine;

struct Sprite {
	uint16 _id;
	uint16 _animId;
	uint16 _zorder;
	Common::Point _pos;
	Graphics::Surface _surface;

	bool contains(const Common::Point &pos) const;
};

struct AnimationEntry {
	uint32 state;
	uint16 op;
	uint16 priority;
	uint16 counter;
	uint16 prevValue;
};

struct Animation {
	Animation(Common::SeekableReadStream *stream, uint16 id, Common::Point basePos, uint32 eventParam);
	~Animation();

	void seekToCurrPos();

	uint16 _id;
	Common::Point _basePos;
	uint32 _eventParam;

	uint32 _state;

	Common::Array<AnimationEntry> _entries;

	uint32 _offset;
	Common::SeekableReadStream *_stream;
};

} // End of namespace Composer

#endif
