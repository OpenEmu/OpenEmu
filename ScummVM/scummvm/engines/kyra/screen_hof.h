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

#ifndef KYRA_SCREEN_HOF_H
#define KYRA_SCREEN_HOF_H

#include "kyra/screen_v2.h"

namespace Kyra {

class KyraEngine_HoF;

class Screen_HoF : public Screen_v2 {
friend class Debugger_v2;
public:
	Screen_HoF(KyraEngine_HoF *vm, OSystem *system);

	// sequence player
	void generateGrayOverlay(const Palette &pal, uint8 *grayOverlay, int factor, int addR, int addG, int addB, int lastColor, bool flag);
	void cmpFadeFrameStep(int srcPage, int srcW, int srcH, int srcX, int srcY, int dstPage,	int dstW, int dstH, int dstX, int dstY, int cmpW, int cmpH, int cmpPage);

private:
	KyraEngine_HoF *_vm;

	static const ScreenDim _screenDimTable[];
	static const int _screenDimTableCount;
};

} // End of namespace Kyra

#endif
