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

#include "mohawk/cstime.h"
#include "mohawk/cstime_graphics.h"
#include "mohawk/resource.h"

#include "common/system.h"
#include "engines/util.h"

namespace Mohawk {

CSTimeGraphics::CSTimeGraphics(MohawkEngine_CSTime *vm) : GraphicsManager(), _vm(vm) {
	_bmpDecoder = new MohawkBitmap();

	initGraphics(640, 480, true);
}

CSTimeGraphics::~CSTimeGraphics() {
	delete _bmpDecoder;
}

void CSTimeGraphics::drawRect(Common::Rect rect, byte color) {
	rect.clip(Common::Rect(640, 480));

	// Useful with debugging. Shows where hotspots are on the screen and whether or not they're active.
	if (!rect.isValidRect() || rect.width() == 0 || rect.height() == 0)
		return;

	Graphics::Surface *screen = _vm->_system->lockScreen();

	screen->frameRect(rect, color);

	_vm->_system->unlockScreen();
}

MohawkSurface *CSTimeGraphics::decodeImage(uint16 id) {
	return _bmpDecoder->decodeImage(_vm->getResource(ID_TBMP, id));
}

Common::Array<MohawkSurface *> CSTimeGraphics::decodeImages(uint16 id) {
	return _bmpDecoder->decodeImages(_vm->getResource(ID_TBMH, id));
}

} // End of namespace Mohawk
