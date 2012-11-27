/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
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

#ifndef PEGASUS_CURSOR_H
#define PEGASUS_CURSOR_H

#include "common/array.h"
#include "common/rect.h"

#include "pegasus/timers.h"

namespace Graphics {
	struct Surface;
}

namespace Pegasus {

// The original cursor code was in the graphics code directly,
// unlike ScummVM where we have the cursor code separate. We're
// going to go with CursorManager here and therefore not inherit
// from the Sprite class.

class Cursor : private Idler {
public:
	Cursor();
	virtual ~Cursor();

	void addCursorFrames(uint16 id);

	void setCurrentFrameIndex(int32 index);
	int32 getCurrentFrameIndex() const;

	void show();
	void hide();
	void hideUntilMoved();
	bool isVisible();

	void getCursorLocation(Common::Point &) const;

protected:
	virtual void useIdleTime();

private:
	struct CursorInfo {
		uint16 tag;
		Common::Point hotspot;
		Graphics::Surface *surface;
		byte *palette;
		uint16 colorCount;
	};

	Common::Point _cursorLocation;
	Common::Array<CursorInfo> _info;
	bool _cursorObscured;
	int _index;

	void loadCursorImage(CursorInfo &cursorInfo);
};

} // End of namespace Pegasus

#endif
