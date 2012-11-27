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

#ifndef MOHAWK_LIVINGBOOKS_GRAPHICS_H
#define MOHAWK_LIVINGBOOKS_GRAPHICS_H

#include "mohawk/graphics.h"

namespace Mohawk {

class MohawkEngine_LivingBooks;

class LBGraphics : public GraphicsManager {
public:
	LBGraphics(MohawkEngine_LivingBooks *vm, uint16 width, uint16 height);
	~LBGraphics();

	void setPalette(uint16 id);
	void copyOffsetAnimImageToScreen(uint16 image, int left = 0, int top = 0);
	bool imageIsTransparentAt(uint16 image, bool useOffsets, int x, int y);

protected:
	MohawkSurface *decodeImage(uint16 id);
	MohawkEngine *getVM() { return (MohawkEngine *)_vm; }

private:
	MohawkBitmap *_bmpDecoder;
	MohawkEngine_LivingBooks *_vm;
};

} // End of namespace Mohawk

#endif
