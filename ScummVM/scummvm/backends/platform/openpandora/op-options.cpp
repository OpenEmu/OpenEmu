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

/*
 * OpenPandora: Options, custom code and hardware stuff.
 *
 */

#include "backends/platform/openpandora/op-options.h"

namespace OP {

enum {
	/* Touchscreen TapMode */
	TAPMODE_LEFT        = 0,
	TAPMODE_RIGHT       = 1,
	TAPMODE_HOVER       = 2,
	TAPMODE_HOVER_DPAD  = 3
};

int tapmodeLevel = TAPMODE_LEFT;

void ToggleTapMode() {
	if (tapmodeLevel == TAPMODE_LEFT) {
		tapmodeLevel = TAPMODE_RIGHT;
	} else if (tapmodeLevel == TAPMODE_RIGHT) {
		tapmodeLevel = TAPMODE_HOVER;
	} else if (tapmodeLevel == TAPMODE_HOVER) {
		tapmodeLevel = TAPMODE_HOVER_DPAD;
	} else if (tapmodeLevel == TAPMODE_HOVER_DPAD) {
		tapmodeLevel = TAPMODE_LEFT;
	} else {
		tapmodeLevel = TAPMODE_LEFT;
	}
}

} /* namespace OP */
