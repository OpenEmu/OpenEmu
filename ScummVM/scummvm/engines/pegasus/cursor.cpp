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

#include "common/events.h"
#include "common/stream.h"
#include "common/system.h"
#include "graphics/cursorman.h"
#include "graphics/surface.h"
#include "graphics/decoders/pict.h"

#include "pegasus/cursor.h"
#include "pegasus/graphics.h"
#include "pegasus/pegasus.h"

namespace Pegasus {

Cursor::Cursor() {
	_cursorObscured = false;
	_index = -1;
	startIdling();
}

Cursor::~Cursor() {
	for (uint32 i = 0; i < _info.size(); i++) {
		if (_info[i].surface) {
			_info[i].surface->free();
			delete _info[i].surface;
		}
		delete[] _info[i].palette;
	}

	stopIdling();
}

void Cursor::addCursorFrames(uint16 id) {
	PegasusEngine *vm = (PegasusEngine *)g_engine;
	Common::SeekableReadStream *cursStream = vm->_resFork->getResource(MKTAG('C', 'u', 'r', 's'), id);
	if (!cursStream)
		error("Could not load cursor frames set %d", id);

	uint16 frameCount = cursStream->readUint16BE();
	for (uint16 i = 0; i < frameCount; i++) {
		CursorInfo info;
		info.tag = cursStream->readUint16BE();
		info.hotspot.x = cursStream->readUint16BE();
		info.hotspot.y = cursStream->readUint16BE();
		info.surface = 0;
		info.palette = 0;
		info.colorCount = 0;
		_info.push_back(info);
	}

	delete cursStream;

	setCurrentFrameIndex(0);
}

void Cursor::setCurrentFrameIndex(int32 index) {
	if (_index != index) {
		_index = index;
		if (index != -1) {
			loadCursorImage(_info[index]);
			CursorMan.replaceCursorPalette(_info[index].palette, 0, _info[index].colorCount);
			CursorMan.replaceCursor((byte *)_info[index].surface->pixels, _info[index].surface->w, _info[index].surface->h, _info[index].hotspot.x, _info[index].hotspot.y, 0);
			((PegasusEngine *)g_engine)->_gfx->markCursorAsDirty();
		}
	}
}

int32 Cursor::getCurrentFrameIndex() const {
	return _index;
}

void Cursor::show() {
	if (!isVisible())
		CursorMan.showMouse(true);

	_cursorObscured = false;
	((PegasusEngine *)g_engine)->_gfx->markCursorAsDirty();
}

void Cursor::hide() {
	CursorMan.showMouse(false);
	setCurrentFrameIndex(0);
	((PegasusEngine *)g_engine)->_gfx->markCursorAsDirty();
}

void Cursor::hideUntilMoved() {
	if (!_cursorObscured) {
		hide();
		_cursorObscured = true;
	}
}

void Cursor::useIdleTime() {
	if (g_system->getEventManager()->getMousePos() != _cursorLocation) {
		_cursorLocation = g_system->getEventManager()->getMousePos();
		if (_index != -1 && _cursorObscured)
			show();
		((PegasusEngine *)g_engine)->_gfx->markCursorAsDirty();
	}
}

void Cursor::getCursorLocation(Common::Point &pt) const {
	pt = _cursorLocation;
}

bool Cursor::isVisible() {
	return CursorMan.isVisible();
}

void Cursor::loadCursorImage(CursorInfo &cursorInfo) {
	if (cursorInfo.surface)
		return;

	cursorInfo.surface = new Graphics::Surface();

	PegasusEngine *vm = (PegasusEngine *)g_engine;
	Common::SeekableReadStream *cicnStream = vm->_resFork->getResource(MKTAG('c', 'i', 'c', 'n'), cursorInfo.tag);

	if (!cicnStream)
		error("Failed to find color icon %d", cursorInfo.tag);

	// PixMap section
	Graphics::PICTDecoder::PixMap pixMap = Graphics::PICTDecoder::readPixMap(*cicnStream);

	// Mask section
	cicnStream->readUint32BE(); // mask baseAddr
	uint16 maskRowBytes = cicnStream->readUint16BE(); // mask rowBytes
	cicnStream->skip(3 * 2); // mask rect
	/* uint16 maskHeight = */ cicnStream->readUint16BE();

	// Bitmap section
	cicnStream->readUint32BE(); // baseAddr
	uint16 rowBytes = cicnStream->readUint16BE();
	cicnStream->readUint16BE(); // top
	cicnStream->readUint16BE(); // left
	uint16 height = cicnStream->readUint16BE(); // bottom
	cicnStream->readUint16BE(); // right

	// Data section
	cicnStream->readUint32BE(); // icon handle
	cicnStream->skip(maskRowBytes * height); // FIXME: maskHeight doesn't work here, though the specs say it should
	cicnStream->skip(rowBytes * height);

	// Palette section
	cicnStream->readUint32BE(); // always 0
	cicnStream->readUint16BE(); // always 0
	cursorInfo.colorCount = cicnStream->readUint16BE() + 1;

	cursorInfo.palette = new byte[cursorInfo.colorCount * 3];
	for (uint16 i = 0; i < cursorInfo.colorCount; i++) {
		cicnStream->readUint16BE();
		cursorInfo.palette[i * 3] = cicnStream->readUint16BE() >> 8;
		cursorInfo.palette[i * 3 + 1] = cicnStream->readUint16BE() >> 8;
		cursorInfo.palette[i * 3 + 2] = cicnStream->readUint16BE() >> 8;
	}

	// PixMap data
	if (pixMap.pixelSize == 8) {
		cursorInfo.surface->create(pixMap.rowBytes, pixMap.bounds.height(), Graphics::PixelFormat::createFormatCLUT8());
		cicnStream->read(cursorInfo.surface->pixels, pixMap.rowBytes * pixMap.bounds.height());

		// While this looks sensible, it actually doesn't work for some cursors
		// (ie. the 'can grab' hand)
		//cursorInfo.surface->w = pixMap.bounds.width();
	} else if (pixMap.pixelSize == 1) {
		cursorInfo.surface->create(pixMap.bounds.width(), pixMap.bounds.height(), Graphics::PixelFormat::createFormatCLUT8());

		for (int y = 0; y < pixMap.bounds.height(); y++) {
			byte *line = (byte *)cursorInfo.surface->getBasePtr(0, y);

			for (int x = 0; x < pixMap.bounds.width();) {
				byte b = cicnStream->readByte();

				for (int i = 0; i < 8; i++) {
					*line++ = ((b & (1 << (7 - i))) != 0) ? 1 : 0;

					if (++x == pixMap.bounds.width())
						break;
				}
			}
		}
	} else {
		error("Unhandled %dbpp cicn images", pixMap.pixelSize);
	}

	delete cicnStream;
}

} // End of namespace Pegasus
