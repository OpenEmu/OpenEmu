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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef KYRA_SCREEN_MR_H
#define KYRA_SCREEN_MR_H

#include "kyra/screen_v2.h"

namespace Kyra {

class KyraEngine_MR;

class Screen_MR : public Screen_v2 {
public:
	Screen_MR(KyraEngine_MR *vm, OSystem *system);
	~Screen_MR();

	int getLayer(int x, int y);

	byte getShapeFlag1(int x, int y);
	byte getShapeFlag2(int x, int y);

	int getDrawLayer(int x, int y);
	int getDrawLayer2(int x, int y, int height);

	void drawFilledBox(int x1, int y1, int x2, int y2, uint8 c1, uint8 c2, uint8 c3);
private:
	static const ScreenDim _screenDimTable[];
	static const int _screenDimTableCount;
};

} // End of namespace Kyra

#endif
