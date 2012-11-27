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

#ifndef MOHAWK_CSTIME_GRAPHICS_H
#define MOHAWK_CSTIME_GRAPHICS_H

#include "mohawk/graphics.h"

namespace Mohawk {

class MohawkEngine_CSTime;

class CSTimeGraphics : public GraphicsManager {
public:
	CSTimeGraphics(MohawkEngine_CSTime *vm);
	~CSTimeGraphics();

	void drawRect(Common::Rect rect, byte color);

protected:
	MohawkSurface *decodeImage(uint16 id);
	Common::Array<MohawkSurface *> decodeImages(uint16 id);
	MohawkEngine *getVM() { return (MohawkEngine *)_vm; }

private:
	MohawkBitmap *_bmpDecoder;
	MohawkEngine_CSTime *_vm;
};

} // End of namespace Mohawk

#endif
