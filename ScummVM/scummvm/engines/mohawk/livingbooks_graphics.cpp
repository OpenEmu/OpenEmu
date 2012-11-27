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

#include "mohawk/resource.h"
#include "mohawk/livingbooks.h"
#include "mohawk/livingbooks_graphics.h"

#include "common/substream.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/palette.h"

namespace Mohawk {

LBGraphics::LBGraphics(MohawkEngine_LivingBooks *vm, uint16 width, uint16 height) : GraphicsManager(), _vm(vm) {
	_bmpDecoder = _vm->isPreMohawk() ? new LivingBooksBitmap_v1() : new MohawkBitmap();

	initGraphics(width, height, true);
}

LBGraphics::~LBGraphics() {
	delete _bmpDecoder;
}

MohawkSurface *LBGraphics::decodeImage(uint16 id) {
	if (_vm->isPreMohawk())
		return _bmpDecoder->decodeImage(_vm->wrapStreamEndian(ID_BMAP, id));

	return _bmpDecoder->decodeImage(_vm->getResource(ID_TBMP, id));
}

void LBGraphics::copyOffsetAnimImageToScreen(uint16 image, int left, int top) {
	MohawkSurface *mhkSurface = findImage(image);

	left -= mhkSurface->getOffsetX();
	top -= mhkSurface->getOffsetY();

	GraphicsManager::copyAnimImageToScreen(image, left, top);
}

bool LBGraphics::imageIsTransparentAt(uint16 image, bool useOffsets, int x, int y) {
	MohawkSurface *mhkSurface = findImage(image);

	if (useOffsets) {
		x += mhkSurface->getOffsetX();
		y += mhkSurface->getOffsetY();
	}

	if (x < 0 || y < 0)
		return true;

	Graphics::Surface *surface = mhkSurface->getSurface();
	if (x >= surface->w || y >= surface->h)
		return true;

	return *(byte *)surface->getBasePtr(x, y) == 0;
}

void LBGraphics::setPalette(uint16 id) {
	// Old Living Books games use the old CTBL-style palette format while newer
	// games use the better tPAL format which can store partial palettes.
	if (_vm->isPreMohawk()) {
		Common::SeekableSubReadStreamEndian *ctblStream = _vm->wrapStreamEndian(ID_CTBL, id);
		uint16 colorCount = ctblStream->readUint16();
		byte *palette = new byte[colorCount * 3];

		for (uint16 i = 0; i < colorCount; i++) {
			palette[i * 3 + 0] = ctblStream->readByte();
			palette[i * 3 + 1] = ctblStream->readByte();
			palette[i * 3 + 2] = ctblStream->readByte();
			ctblStream->readByte();
		}

		delete ctblStream;

		_vm->_system->getPaletteManager()->setPalette(palette, 0, colorCount);
		delete[] palette;
	} else {
		GraphicsManager::setPalette(id);
	}
}

} // End of namespace Mohawk
