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

#if defined(MAEMO)

#ifndef PLATFORM_SDL_MAEMO_COMMON_H
#define PLATFORM_SDL_MAEMO_COMMON_H

namespace Maemo {

enum ModelType {
	kModelType770 = 1 << 0,
	kModelTypeN800 = 1 << 1,
	kModelTypeN810 = 1 << 2,
	kModelTypeN900 = 1 << 3,
	kModelTypeInvalid = 0
};

struct Model {
	const char *hwId;
	ModelType modelType;
	const char *hwAlias;
	bool hasHwKeyboard;
	bool hasMenuKey;
};

enum CustomEventType {
	kEventClickMode = 1,
	kEventInvalid = 0
};

} // namespace Maemo

#endif // ifndef PLATFORM_SDL_MAEMO_COMMON_H

#endif // if defined(MAEMO)
