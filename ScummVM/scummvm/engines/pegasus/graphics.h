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

#ifndef PEGASUS_GRAPHICS_H
#define PEGASUS_GRAPHICS_H

#include "common/rect.h"
#include "common/str.h"
#include "common/system.h"
#include "graphics/surface.h"

#include "pegasus/constants.h"
#include "pegasus/pegasus.h"
#include "pegasus/util.h"

namespace Pegasus {

class Cursor;
class DisplayElement;
class PegasusEngine;
class ScreenFader;

class GraphicsManager {
friend class Cursor;
public:
	GraphicsManager(PegasusEngine *vm);
	~GraphicsManager();

	void addDisplayElement(DisplayElement *element);
	void removeDisplayElement(DisplayElement *element);
	void invalRect(const Common::Rect &rect);
	DisplayOrder getBackOfActiveLayer() const { return _backLayer; }
	DisplayOrder getFrontOfActiveLayer() const { return _frontLayer; }
	void updateDisplay();
	Graphics::Surface *getCurSurface() { return _curSurface; }
	void setCurSurface(Graphics::Surface *surface) { _curSurface = surface; }
	Graphics::Surface *getWorkArea() { return &_workArea; }
	void clearScreen();
	DisplayElement *findDisplayElement(const DisplayElementID id);
	void shakeTheWorld(TimeValue time, TimeScale scale);
	void enableErase();
	void disableErase();
	void enableUpdates();
	void disableUpdates();

	// These default to black
	void doFadeOutSync(const TimeValue = kOneSecondPerThirtyTicks, const TimeScale = kThirtyTicksPerSecond, bool isBlack = true);
	void doFadeInSync(const TimeValue = kOneSecondPerThirtyTicks, const TimeScale = kThirtyTicksPerSecond, bool isBlack = true);

protected:
	void markCursorAsDirty();

private:
	PegasusEngine *_vm;

	bool _modifiedScreen, _erase;
	Common::Rect _dirtyRect;
	DisplayOrder _backLayer, _frontLayer;
	DisplayElement *_firstDisplayElement, *_lastDisplayElement;
	Graphics::Surface _workArea, *_curSurface;

	// Shake Shake Shake!
	static const int kMaxShakeOffsets = 17;
	Common::Point _shakeOffsets[kMaxShakeOffsets];
	void newShakePoint(int32 index1, int32 index2, int32 maxRadius);

	bool _updatesEnabled;
	ScreenFader *_screenFader;
};

} // End of namespace Pegasus

#endif
