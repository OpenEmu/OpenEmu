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

#include "common/iff_container.h"
#include "common/textconsole.h"

#include "parallaction/disk.h"
#include "parallaction/graphics.h"

namespace Parallaction {

void ILBMLoader::setupBuffer(uint32 w, uint32 h) {
	_intBuffer = 0;
	switch (_bodyMode) {
	case BODYMODE_SURFACE:
		if (!_surf) {
			_surf = new Graphics::Surface;
			assert(_surf);
		}
		_surf->create(w, h, Graphics::PixelFormat::createFormatCLUT8());
		_mode  = Graphics::ILBMDecoder::ILBM_UNPACK_PLANES;
		_intBuffer = (byte *)_surf->pixels;
		break;

	case BODYMODE_MASKBUFFER:
		if (!_maskBuffer) {
			_maskBuffer = new MaskBuffer;
			assert(_maskBuffer);
		}
		_maskBuffer->create(w, h);
		_mode  = Graphics::ILBMDecoder::ILBM_2_PACK_PLANES;
		_intBuffer = _maskBuffer->data;
		break;

	case BODYMODE_PATHBUFFER:
		if (!_pathBuffer) {
			_pathBuffer = new PathBuffer;
			assert(_pathBuffer);
		}
		_pathBuffer->create(w, h);
		_mode  = Graphics::ILBMDecoder::ILBM_1_PACK_PLANES;
		_intBuffer = _pathBuffer->data;
		break;

	default:
		error("Invalid bodyMode '%i' for ILBMLoader", _bodyMode);
		break;
	}
}

bool ILBMLoader::callback(Common::IFFChunk &chunk) {
	switch (chunk._type) {
	case ID_BMHD:
		_decoder.loadHeader(chunk._stream);
		break;

	case ID_CMAP:
		if (_palette) {
			chunk._stream->read(_palette, chunk._size);
		}
		break;

	case ID_CRNG:
		if (_crng) {
			PaletteFxRange *ptr = &_crng[_numCRNG];
			chunk._stream->read((byte *)ptr, chunk._size);
			ptr->_timer = FROM_BE_16(ptr->_timer);
			ptr->_step = FROM_BE_16(ptr->_step);
			ptr->_flags = FROM_BE_16(ptr->_flags);
			++_numCRNG;
		}
		break;

	case ID_BODY:
		setupBuffer(_decoder._header.width, _decoder._header.height);
		assert(_intBuffer);
		_decoder.loadBitmap(_mode, _intBuffer, chunk._stream);
		return true;	// stop the parser
	}

	return false;
}

void ILBMLoader::load(Common::ReadStream *in, bool disposeStream) {
	Common::IFFParser parser(in, disposeStream);
	Common::Functor1Mem< Common::IFFChunk&, bool, ILBMLoader > c(this, &ILBMLoader::callback);
	parser.parse(c);
}

ILBMLoader::ILBMLoader(uint32 bodyMode, byte *palette, PaletteFxRange *crng) {
	_bodyMode = bodyMode;
	_surf = 0;
	_maskBuffer = 0;
	_pathBuffer = 0;
	_palette = palette;
	_crng = crng;
	_numCRNG = 0;
}

ILBMLoader::ILBMLoader(Graphics::Surface *surf, byte *palette, PaletteFxRange *crng) {
	_bodyMode = ILBMLoader::BODYMODE_SURFACE;
	_surf = surf;
	_palette = palette;
	_crng = crng;
	_numCRNG = 0;
}

ILBMLoader::ILBMLoader(MaskBuffer *buffer) {
	_bodyMode = ILBMLoader::BODYMODE_MASKBUFFER;
	_maskBuffer = buffer;
	_palette = 0;
	_crng = 0;
	_numCRNG = 0;
}

ILBMLoader::ILBMLoader(PathBuffer *buffer) {
	_bodyMode = ILBMLoader::BODYMODE_PATHBUFFER;
	_pathBuffer = buffer;
	_palette = 0;
	_crng = 0;
	_numCRNG = 0;
}



}
