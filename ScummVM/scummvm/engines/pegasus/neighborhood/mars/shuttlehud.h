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

#ifndef PEGASUS_NEIGHBORHOOD_MARS_SHUTTLEHUD_H
#define PEGASUS_NEIGHBORHOOD_MARS_SHUTTLEHUD_H

#include "pegasus/elements.h"
#include "pegasus/timers.h"

namespace Pegasus {

class ShuttleHUD : public DisplayElement, public Idler {
public:
	ShuttleHUD();

	void showTargetGrid();
	void hideTargetGrid();

	void initShuttleHUD();
	void cleanUpShuttleHUD();

	bool isTargetLocked() { return _targetLocked; }

	void draw(const Common::Rect &);

protected:
	void useIdleTime();
	void lockOnTarget();
	void unlockOnTarget();
	void drawOneBitImageOr(Graphics::Surface *, const uint16 *, int, const Common::Rect &, uint32);

	bool _targetLocked;
	uint32 _lightGreen, _gridDarkGreen, _lockDarkGreen1, _lockDarkGreen2;
};

} // End of namespace Pegasus

#endif
